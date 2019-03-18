#include "WasmModule.h"

#include <util/bytes.h>
#include <util/logging.h>
#include <util/config.h>

#include <boost/filesystem.hpp>
#include <fcntl.h>
#include <math.h>
#include <dirent.h>
#include <netdb.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/uio.h>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/RuntimeData.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Inline/FloatComponents.h>

#include <stdarg.h>

using namespace WAVM;

/**
 * USEFUL NOTES
 *
 * Any syscalls here must be included in the musl port at:
 *   - musl/arch/wasm32/syscall_arch.h and
 *   - musl/arch/wasm32/libc.imports
 *
 * The syscall numbers can be found in musl/arch/wasm32/bits/syscall.h.in
 *
 * Any standard library functions can be found in files appropriately named
 * in musl/src. E.g. getaddrinfo is defined in musl/src/network/getaddinfo.c
 *
 * Once implemented there, they will be referenced in the wasm files, and stubbed at runtime if they
 * don't exist in here.
 *
 * Any structs passed as arguments must be re-implemented here with the following mappings
 * (respecting signed/ unsigned):
 *
 * short = I16/U16
 * long = I32/U32
 * int = I32/U32
 * char = U8
 * pointers = Uptr
 * size_t = I32
 *
 */

namespace wasm {
    static thread_local U32 EMSCRIPTEN_ERRNO_LOCATION = 0;

    void setEmscriptenErrnoLocation(U32 value) {
        EMSCRIPTEN_ERRNO_LOCATION = value;
    }

    static Intrinsics::Module envModule;
    static Intrinsics::Module emEnvModule;
    static Intrinsics::Module emAsm2wasmModule;
    static Intrinsics::Module emGlobalModule;

    Intrinsics::Module &getIntrinsicModule_env() {
        return envModule;
    }

    Intrinsics::Module &getIntrinsicModule_emEnv() {
        return emEnvModule;
    }

    Intrinsics::Module &getIntrinsicModule_emAsm2wasm() {
        return emAsm2wasmModule;
    }

    Intrinsics::Module &getIntrinsicModule_emGlobal() {
        return emGlobalModule;
    }

    static const char *FAKE_NAME = "faasm";
    static const char *FAKE_PASSWORD = "foobar123";
    static const char *FAKE_HOME = "/";
    static const int FAKE_UID = 1;
    static const int FAKE_GID = 1;
    static const char *FALSE_ROOT = "/usr/local/faasm/runtime_root";
    static const char *HOSTS_FILE = "/usr/local/faasm/net/hosts";
    static const char *RESOLV_FILE = "/usr/local/faasm/net/resolv.conf";

    U32 *emscriptenArgs(U32 syscallNo, I32 argsPtr, int argCount) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        U32 *args = Runtime::memoryArrayPtr<U32>(memoryPtr, (Uptr) argsPtr, argCount);

        return args;
    }

    static U32 coerce32bitAddress(Runtime::Memory *memory, Uptr address) {
        if (address >= UINT32_MAX) {
            throwException(Runtime::ExceptionTypes::outOfBoundsMemoryAccess, {asObject(memory), U64(address)});
        }
        return (U32) address;
    }

    // Thread-local variables to isolate bits of environment
    static thread_local std::set<int> openFds;

    void getBytesFromWasm(I32 dataPtr, I32 dataLen, uint8_t *buffer) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        U8 *data = Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr) dataPtr, (Uptr) dataLen);

        std::copy(data, data + dataLen, buffer);
    }

    std::vector<uint8_t> getBytesFromWasm(I32 dataPtr, I32 dataLen) {
        std::vector<uint8_t> bytes(dataLen);
        getBytesFromWasm(dataPtr, dataLen, bytes.data());
        return bytes;
    }

    std::string getStringFromWasm(I32 strPtr) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        char *key = &Runtime::memoryRef<char>(memoryPtr, (Uptr) strPtr);
        std::string str(key);

        return str;
    }

    std::pair<std::string, std::string> getUserKeyPairFromWasm(I32 keyPtr) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        char *key = &Runtime::memoryRef<char>(memoryPtr, (Uptr) keyPtr);

        const message::Message *call = getExecutingCall();
        return std::pair<std::string, std::string>(call->user(), key);
    }

    std::string maskPath(std::string originalPath) {
        boost::filesystem::path p(FALSE_ROOT);
        p.append(originalPath);
        return p.string();
    }

    std::string getMaskedPathFromWasm(I32 strPtr) {
        const std::string originalPath = getStringFromWasm(strPtr);

        return maskPath(originalPath);
    }


    // ---------------------------
    // System-related structs
    // ---------------------------

    // Timing
    struct wasm_timespec {
        I32 tv_sec;
        I32 tv_nsec;
    };

    // Timing
    struct wasm_timeval {
        I32 tv_sec;
        I32 tv_usec;
    };

    // I/O
    struct wasm_iovec {
        U32 iov_base;
        U32 iov_len;
    };

    /** Socket-related struct (see https://beej.us/guide/bgnet/html/multi/sockaddr_inman.html) */
    struct wasm_sockaddr {
        U16 sa_family;
        U8 sa_data[14];
    };

    /** Translates a wasm sockaddr into a native sockaddr */
    sockaddr getSockAddr(I32 addrPtr) {
        auto addr = &Runtime::memoryRef<wasm_sockaddr>(getExecutingModule()->defaultMemory, (Uptr) addrPtr);

        sockaddr sa = {.sa_family = addr->sa_family};

        std::copy(addr->sa_data, addr->sa_data + 14, sa.sa_data);
        return sa;
    }

    // Taken from arch/wasm32/bits/stat.h in the musl port
    struct wasm_stat {
        U32 st_dev;
        I32 __st_dev_padding;
        I32 __st_ino_truncated;
        U32 st_mode;
        U32 st_nlink;
        U32 st_uid;
        U32 st_gid;
        U32 st_rdev;
        I32 __st_rdev_padding;
        I32 st_size;
        I32 st_blksize;
        I32 st_blocks;
        struct wasm_timespec st_atim;
        struct wasm_timespec st_mtim;
        struct wasm_timespec st_ctim;
        U32 st_ino;
    };

    /** Translates a native stat to a wasm stat */
    void writeNativeStatToWasmStat(struct stat64 *nativeStatPtr, I32 wasmStatPtr) {
        auto wasmHostPtr = &Runtime::memoryRef<wasm_stat>(getExecutingModule()->defaultMemory, (Uptr) wasmStatPtr);

        // Support *some* details from the host but not all
        wasmHostPtr->st_dev = nativeStatPtr->st_dev;
        wasmHostPtr->st_mode = nativeStatPtr->st_mode;
        wasmHostPtr->st_nlink = nativeStatPtr->st_nlink;

        wasmHostPtr->st_uid = nativeStatPtr->st_uid;
        wasmHostPtr->st_gid = nativeStatPtr->st_gid;

        wasmHostPtr->st_rdev = nativeStatPtr->st_rdev;
        wasmHostPtr->st_size = nativeStatPtr->st_size;

        wasmHostPtr->st_blksize = nativeStatPtr->st_blksize;
        wasmHostPtr->st_blocks = nativeStatPtr->st_blocks;

        wasmHostPtr->st_atim.tv_sec = nativeStatPtr->st_atim.tv_sec;
        wasmHostPtr->st_atim.tv_nsec = nativeStatPtr->st_atim.tv_nsec;

        wasmHostPtr->st_mtim.tv_sec = nativeStatPtr->st_mtim.tv_sec;
        wasmHostPtr->st_mtim.tv_nsec = nativeStatPtr->st_mtim.tv_nsec;

        wasmHostPtr->st_ctim.tv_sec = nativeStatPtr->st_ctim.tv_sec;
        wasmHostPtr->st_ctim.tv_nsec = nativeStatPtr->st_ctim.tv_nsec;

        wasmHostPtr->st_ino = nativeStatPtr->st_ino;
    }

    /**
     * Note that char pointers are U32
     */
    struct wasm_passwd {
        U32 pw_name;
        U32 pw_passwd;
        U32 pw_uid;
        U32 pw_gid;
        U32 pw_gecos;
        U32 pw_dir;
        U32 pw_shell;
    };

    /**
     * Note, this struct type works with emscripten, but may not work with musl implementations
     */
    struct wasm_dirent64 {
        U32 d_ino;
        I32 d_off;
        U16 d_reclen;
        U8 d_type;
        U8 d_name[256];
    };

    // ------------------------
    // FAASM-specific
    // ------------------------
    std::shared_ptr<state::StateKeyValue> getStateKV(I32 keyPtr, size_t size) {
        const std::pair<std::string, std::string> userKey = getUserKeyPairFromWasm(keyPtr);
        state::State &s = state::getGlobalState();
        auto kv = s.getKV(userKey.first, userKey.second, size);

        return kv;
    }

    std::shared_ptr<state::StateKeyValue> getStateKVRead(I32 keyPtr, size_t size, int async) {
        auto kv = getStateKV(keyPtr, size);

        bool isAsync = async == 1;
        kv->pull(isAsync);

        return kv;
    }

    void s__faasm_push_state(I32 keyPtr) {
        util::getLogger()->debug("S - push_state - {}", keyPtr);

        auto kv = getStateKV(keyPtr, 0);
        kv->pushFull();
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_push_state", void, __faasm_push_state, I32 keyPtr) {
        s__faasm_push_state(keyPtr);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___faasm_push_state", void, ___faasm_push_state, I32 keyPtr) {
        s__faasm_push_state(keyPtr);
    }

    void s__faasm_push_state_partial(I32 keyPtr) {
        util::getLogger()->debug("S - push_state_partial - {}", keyPtr);

        auto kv = getStateKV(keyPtr, 0);
        kv->pushPartial();
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_push_state_partial", void, __faasm_push_state_partial, I32 keyPtr) {
        s__faasm_push_state_partial(keyPtr);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___faasm_push_state_partial", void, ___faasm_push_state_partial, I32 keyPtr) {
        s__faasm_push_state_partial(keyPtr);
    }

    void s__faasm_lock_state_read(I32 keyPtr) {
        util::getLogger()->debug("S - lock_state_read - {}", keyPtr);

        auto kv = getStateKV(keyPtr, 0);
        kv->lockRead();
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_lock_state_read", void, __faasm_lock_state_read, I32 keyPtr) {
        s__faasm_lock_state_read(keyPtr);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___faasm_lock_state_read", void, ___faasm_lock_state_read, I32 keyPtr) {
        s__faasm_lock_state_read(keyPtr);
    }

    void s__faasm_unlock_state_read(I32 keyPtr) {
        util::getLogger()->debug("S - unlock_state_read - {}", keyPtr);

        auto kv = getStateKV(keyPtr, 0);
        kv->unlockRead();
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_unlock_state_read", void, __faasm_unlock_state_read, I32 keyPtr) {
        s__faasm_unlock_state_read(keyPtr);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___faasm_unlock_state_read", void, ___faasm_unlock_state_read, I32 keyPtr) {
        s__faasm_unlock_state_read(keyPtr);
    }

    void s__faasm_lock_state_write(I32 keyPtr) {
        util::getLogger()->debug("S - lock_state_write - {}", keyPtr);

        auto kv = getStateKV(keyPtr, 0);
        kv->lockWrite();
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_lock_state_write", void, __faasm_lock_state_write, I32 keyPtr) {
        s__faasm_lock_state_write(keyPtr);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___faasm_lock_state_write", void, ___faasm_lock_state_write, I32 keyPtr) {
        s__faasm_lock_state_write(keyPtr);
    }

    void s__faasm_unlock_state_write(I32 keyPtr) {
        util::getLogger()->debug("S - unlock_state_write - {}", keyPtr);

        auto kv = getStateKV(keyPtr, 0);
        kv->unlockWrite();
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_unlock_state_write", void, __faasm_unlock_state_write, I32 keyPtr) {
        s__faasm_unlock_state_write(keyPtr);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___faasm_unlock_state_write", void, ___faasm_unlock_state_write, I32 keyPtr) {
        s__faasm_unlock_state_write(keyPtr);
    }

    void s__faasm_write_state(I32 keyPtr, I32 dataPtr, I32 dataLen, I32 async) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - write_state - {} {} {} {}", keyPtr, dataPtr, dataLen, async);

        auto kv = getStateKV(keyPtr, dataLen);

        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        U8 *data = Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr) dataPtr, (Uptr) dataLen);

        logger->debug("Writing state length {} to key {}", dataLen, kv->key);
        kv->set(data);

        // Push if synchronous
        if (async == 0) {
            kv->pushFull();
        }
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_write_state", void, __faasm_write_state,
                              I32 keyPtr, I32 dataPtr, I32 dataLen, I32 async) {
        s__faasm_write_state(keyPtr, dataPtr, dataLen, async);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___faasm_write_state", void, ___faasm_write_state,
                              I32 keyPtr, I32 dataPtr, I32 dataLen, I32 async) {
        s__faasm_write_state(keyPtr, dataPtr, dataLen, async);
    }

    void s__faasm_write_state_offset(I32 keyPtr, I32 totalLen, I32 offset, I32 dataPtr, I32 dataLen, I32 async) {
        util::getLogger()->debug("S - write_state_offset - {} {} {} {} {} {}", keyPtr, totalLen, offset, dataPtr,
                                 dataLen, async);

        auto kv = getStateKV(keyPtr, totalLen);

        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        U8 *data = Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr) dataPtr, (Uptr) dataLen);

        kv->setSegment(offset, data, dataLen);

        // Push if synchronous
        if (async == 0) {
            kv->pushPartial();
        }
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_write_state_offset", void, __faasm_write_state_offset,
                              I32 keyPtr, I32 totalLen, I32 offset, I32 dataPtr, I32 dataLen, I32 async) {
        s__faasm_write_state_offset(keyPtr, totalLen, offset, dataPtr, dataLen, async);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___faasm_write_state_offset", void, ___faasm_write_state_offset,
                              I32 keyPtr, I32 totalLen, I32 offset, I32 dataPtr, I32 dataLen, I32 async) {
        s__faasm_write_state_offset(keyPtr, totalLen, offset, dataPtr, dataLen, async);
    }

    void s__faasm_read_state(I32 keyPtr, I32 bufferPtr, I32 bufferLen, I32 async) {
        util::getLogger()->debug("S - read_state - {} {} {} {}", keyPtr, bufferPtr, bufferLen, async);

        auto kv = getStateKVRead(keyPtr, bufferLen, async);

        // Copy to straight to buffer
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        U8 *buffer = Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr) bufferPtr, (Uptr) bufferLen);
        kv->get(buffer);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_read_state", void, __faasm_read_state,
                              I32 keyPtr, I32 bufferPtr, I32 bufferLen, I32 async) {
        s__faasm_read_state(keyPtr, bufferPtr, bufferLen, async);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___faasm_read_state", void, ___faasm_read_state,
                              I32 keyPtr, I32 bufferPtr, I32 bufferLen, I32 async) {
        s__faasm_read_state(keyPtr, bufferPtr, bufferLen, async);
    }

    I32 s__faasm_read_state_ptr(I32 keyPtr, I32 totalLen, I32 async) {
        util::getLogger()->debug("S - read_state - {} {} {}", keyPtr, totalLen, async);

        auto kv = getStateKVRead(keyPtr, totalLen, async);

        // Map shared memory
        WasmModule *module = getExecutingModule();
        U32 wasmPtr = module->mmapKey(kv, totalLen);

        return wasmPtr;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_read_state_ptr", I32, __faasm_read_state_ptr,
                              I32 keyPtr, I32 totalLen, I32 async) {
        return s__faasm_read_state_ptr(keyPtr, totalLen, async);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___faasm_read_state_ptr", I32, ___faasm_read_state_ptr,
                              I32 keyPtr, I32 totalLen, I32 async) {
        return s__faasm_read_state_ptr(keyPtr, totalLen, async);
    }

    void s__faasm_read_state_offset(I32 keyPtr, I32 totalLen, I32 offset, I32 bufferPtr, I32 bufferLen, I32 async) {
        util::getLogger()->debug("S - read_state_offset - {} {} {} {} {}", keyPtr, totalLen, offset, bufferPtr,
                                 bufferLen);

        auto kv = getStateKVRead(keyPtr, totalLen, async);

        // Copy to straight to buffer
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        U8 *buffer = Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr) bufferPtr, (Uptr) bufferLen);
        kv->getSegment(offset, buffer, bufferLen);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_read_state_offset", void, __faasm_read_state_offset,
                              I32 keyPtr, I32 totalLen, I32 offset, I32 bufferPtr, I32 bufferLen, I32 async) {
        s__faasm_read_state_offset(keyPtr, totalLen, offset, bufferPtr, bufferLen, async);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___faasm_read_state_offset", void, ___faasm_read_state_offset,
                              I32 keyPtr, I32 totalLen, I32 offset, I32 bufferPtr, I32 bufferLen, I32 async) {
        s__faasm_read_state_offset(keyPtr, totalLen, offset, bufferPtr, bufferLen, async);
    }

    I32 s__faasm_read_state_offset_ptr(I32 keyPtr, I32 totalLen, I32 offset, I32 len, I32 async) {
        util::getLogger()->debug("S - read_state_offset_ptr - {} {} {} {} {}", keyPtr, totalLen, offset, len, async);

        auto kv = getStateKVRead(keyPtr, totalLen, async);

        // Map whole key in shared memory
        WasmModule *module = getExecutingModule();
        U32 wasmPtr = module->mmapKey(kv, totalLen);

        // Return pointer to offset region
        I32 offsetPtr = wasmPtr + offset;
        return offsetPtr;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_read_state_offset_ptr", I32, __faasm_read_state_offset_ptr,
                              I32 keyPtr, I32 totalLen, I32 offset, I32 len, I32 async) {
        return s__faasm_read_state_offset_ptr(keyPtr, totalLen, offset, len, async);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___faasm_read_state_offset_ptr", I32, ___faasm_read_state_offset_ptr,
                              I32 keyPtr, I32 totalLen, I32 offset, I32 len, I32 async) {
        return s__faasm_read_state_offset_ptr(keyPtr, totalLen, offset, len, async);
    }

    void s__faasm_flag_state_dirty(I32 keyPtr, I32 totalLen) {
        util::getLogger()->debug("S - __faasm_flag_state_dirty - {} {}", keyPtr, totalLen);

        auto kv = getStateKV(keyPtr, totalLen);
        kv->flagFullValueDirty();
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_flag_state_dirty", void, __faasm_flag_state_dirty,
                              I32 keyPtr, I32 totalLen) {
        s__faasm_flag_state_dirty(keyPtr, totalLen);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___faasm_flag_state_dirty", void, ___faasm_flag_state_dirty,
                              I32 keyPtr, I32 totalLen) {
        s__faasm_flag_state_dirty(keyPtr, totalLen);
    }

    void s__faasm_flag_state_offset_dirty(I32 keyPtr, I32 totalLen, I32 offset, I32 len) {
        // Avoid heavy logging
        //        util::getLogger()->debug("S - __faasm_flag_state_offset_dirty - {} {} {} {}", keyPtr, totalLen, offset,
        //                                 len);

        auto kv = getStateKV(keyPtr, totalLen);
        kv->flagSegmentDirty(offset, len);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_flag_state_offset_dirty", void, __faasm_flag_state_offset_dirty,
                              I32 keyPtr, I32 totalLen, I32 offset, I32 len) {
        s__faasm_flag_state_offset_dirty(keyPtr, totalLen, offset, len);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___faasm_flag_state_offset_dirty", void, ___faasm_flag_state_offset_dirty,
                              I32 keyPtr, I32 totalLen, I32 offset, I32 len) {
        s__faasm_flag_state_offset_dirty(keyPtr, totalLen, offset, len);
    }

    I32 s__faasm_read_input(I32 bufferPtr, I32 bufferLen) {
        util::getLogger()->debug("S - read_input - {} {}", bufferPtr, bufferLen);

        // Get the input
        message::Message *call = getExecutingCall();
        std::vector<uint8_t> inputBytes = util::stringToBytes(call->inputdata());

        // If nothing, return nothing
        if (inputBytes.empty()) {
            return 0;
        }

        // Write to the wasm buffer
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        U8 *buffer = Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr) bufferPtr, (Uptr) bufferLen);

        int inputSize = util::safeCopyToBuffer(inputBytes, buffer, bufferLen);
        return inputSize;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_read_input", I32, __faasm_read_input, I32 bufferPtr, I32 bufferLen) {
        return s__faasm_read_input(bufferPtr, bufferLen);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___faasm_read_input", I32, ___faasm_read_input, I32 bufferPtr, I32 bufferLen) {
        return s__faasm_read_input(bufferPtr, bufferLen);
    }

    void s__faasm_write_output(I32 outputPtr, I32 outputLen) {
        util::getLogger()->debug("S - write_output - {} {}", outputPtr, outputLen);

        std::vector<uint8_t> outputData = getBytesFromWasm(outputPtr, outputLen);
        message::Message *call = getExecutingCall();
        call->set_outputdata(outputData.data(), outputData.size());
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_write_output", void, __faasm_write_output, I32 outputPtr, I32 outputLen) {
        s__faasm_write_output(outputPtr, outputLen);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___faasm_write_output", void, ___faasm_write_output, I32 outputPtr,
                              I32 outputLen) {
        s__faasm_write_output(outputPtr, outputLen);
    }

    void s__faasm_chain_function(I32 namePtr, I32 inputDataPtr, I32 inputDataLen) {
        util::getLogger()->debug("S - chain_function - {} {} {}", namePtr, inputDataPtr, inputDataLen);

        message::Message *call = getExecutingCall();
        CallChain *callChain = getExecutingCallChain();
        std::string funcName = getStringFromWasm(namePtr);
        const std::vector<uint8_t> inputData = getBytesFromWasm(inputDataPtr, inputDataLen);

        // Add this to the chain of calls
        callChain->addCall(call->user(), funcName, inputData);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_chain_function", void, __faasm_chain_function,
                              I32 namePtr, I32 inputDataPtr, I32 inputDataLen) {
        s__faasm_chain_function(namePtr, inputDataPtr, inputDataLen);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___faasm_chain_function", void, ___faasm_chain_function,
                              I32 namePtr, I32 inputDataPtr, I32 inputDataLen) {
        s__faasm_chain_function(namePtr, inputDataPtr, inputDataLen);
    }

    void s__faasm_snapshot_memory(I32 keyPtr) {
        util::getLogger()->debug("S - snapshot_memory - {}", keyPtr);
        WasmModule *module = getExecutingModule();

        std::string key = getStringFromWasm(keyPtr);
        module->snapshotFullMemory(key.c_str());
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_snapshot_memory", void, __faasm_snapshot_memory, I32 keyPtr) {
        s__faasm_snapshot_memory(keyPtr);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___faasm_snapshot_memory", void, ___faasm_snapshot_memory, I32 keyPtr) {
        s__faasm_snapshot_memory(keyPtr);
    }

    void s__faasm_restore_memory(I32 keyPtr) {
        util::getLogger()->debug("S - restore_memory - {}", keyPtr);

        WasmModule *module = getExecutingModule();

        std::string key = getStringFromWasm(keyPtr);
        module->restoreFullMemory(key.c_str());
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_restore_memory", void, __faasm_restore_memory, I32 keyPtr) {
        s__faasm_restore_memory(keyPtr);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___faasm_restore_memory", void, ___faasm_restore_memory, I32 keyPtr) {
        s__faasm_restore_memory(keyPtr);
    }

    // ------------------------
    // I/O - supported
    // ------------------------

    void checkThreadOwnsFd(int fd) {
        const std::shared_ptr<spdlog::logger> logger = util::getLogger();
        bool isNotOwned = openFds.find(fd) == openFds.end();

        if (fd == STDIN_FILENO) {
            logger->warn("Process interacting with stdin");

            // TODO - this is a hack to allow Python to work.
            // throw std::runtime_error("Attempt to interact with stdin");
        } else if (fd == STDOUT_FILENO) {
            // Can allow stdout/ stderr through
            // logger->debug("Process interacting with stdout", fd);
        } else if (fd == STDERR_FILENO) {
            // logger->debug("Process interacting with stderr", fd);
        } else if (isNotOwned) {
            logger->error("fd not owned by this thread (%i)\n", fd);
            throw std::runtime_error("fd not owned by this function");
        }
    }

    /** Whitelist specific files to allow open and read-only */
    I32 s__syscall_open(I32 pathPtr, I32 flags, I32 mode) {
        const std::shared_ptr<spdlog::logger> logger = util::getLogger();
        const std::string path = getStringFromWasm(pathPtr);
        logger->debug("S - open - {} {} {}", path, flags, mode);

        // Check if this is a valid path. Return a read-only handle to the file if so
        int fd;
        std::string fakePath;
        if (path == "/etc/hosts") {
            logger->debug("Opening dummy /etc/hosts");
            fd = open(HOSTS_FILE, 0, 0);

        } else if (path == "/etc/resolv.conf") {
            logger->debug("Opening dummy /etc/resolv.conf");
            fd = open(RESOLV_FILE, 0, 0);

        } else if (path == "/dev/urandom") {
            //TODO avoid use of system-wide urandom
            logger->debug("Opening /dev/urandom");
            fd = open("/dev/urandom", 0, 0);

        } else if (path == "pyvenv.cfg") {
            logger->debug("Forcing non-existent pyvenv.cfg");
            return -ENOENT;
        } else {
            fakePath = maskPath(path.c_str());
            logger->debug("Opening {}", fakePath);

            //TODO - need some sanity checks around these flags and mode
            fd = open(fakePath.c_str(), flags, mode);
        }

        if (fd > 0) {
            openFds.insert(fd);
        } else {
            // This is an error of some form
            if (!fakePath.empty()) {
                logger->error("Failed on open - {} (masked {})", path, fakePath);
            } else {
                logger->error("Failed on open - {}", path);
            }
        }

        return fd;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_open", I32, __syscall_open, I32 pathPtr, I32 flags, I32 mode) {
        return s__syscall_open(pathPtr, flags, mode);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall5", I32, ___syscall5, I32 syscallNo, I32 argsPtr) {
        U32 *args = emscriptenArgs(syscallNo, argsPtr, 3);
        return s__syscall_open(args[0], args[1], args[2]);
    }

    I32 s__syscall_dup(I32 oldFd) {
        util::getLogger()->debug("S - dup - {}", oldFd);

        checkThreadOwnsFd(oldFd);

        int newFd = dup(oldFd);
        openFds.insert(newFd);

        return newFd;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_dup", I32, __syscall_dup, I32 oldFd) {
        return s__syscall_dup(oldFd);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall41", I32, ___syscall41, I32 syscallNo, I32 argsPtr) {
        U32 *args = emscriptenArgs(syscallNo, argsPtr, 1);
        return s__syscall_dup(args[0]);
    }

    I32 s__syscall_fcntl64(I32 fd, I32 cmd, I32 c) {
        util::getLogger()->debug("S - fcntl64 - {} {} {}", fd, cmd, c);

        checkThreadOwnsFd(fd);

        // Allow for now
        // TODO - is this ok?
        fcntl(fd, cmd, c);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_fcntl64", I32, __syscall_fcntl64,
                              I32 fd, I32 cmd, I32 c) {
        return s__syscall_fcntl64(fd, cmd, c);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall221", I32, ___syscall221,
                              I32 syscallNo, I32 argsPtr) {
        U32 *args = emscriptenArgs(syscallNo, argsPtr, 3);
        return s__syscall_fcntl64(args[0], args[1], args[2]);
    }

    /**
     * This function is tricky to implement as it's iterating through an unknown number of
     * files in the directory. We will be running two loops, one to fill up the given buffer,
     * and a nested loop to iterate through the native directory listing.
     *
     * We try to be conservative but will throw an exception if things aren't right. A bug here
     * can be hard to find.
     */
    I32 s__syscall_getdents64(U32 fd, I32 wasmDirentBuf, U32 wasmDirentBufLen) {
        util::getLogger()->debug("S - getdents64 - {} {} {}", fd, wasmDirentBuf, wasmDirentBufLen);

        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        U8 *hostWasmDirentBuf = &Runtime::memoryRef<U8>(memoryPtr, (Uptr) wasmDirentBuf);

        // Work out how many dirents we can fit (rounding down)
        size_t wasmDirentSize = sizeof(wasm_dirent64);

        // Create a small native buffer (can't overshoot the wasm offset)
        int nativeBufLen = 60;

        U32 wasmBytesRead;
        int wasmDirentCount = 0;

        // Here we will iterate getting native dirents until we've filled up the wasm buffer supplied
        for (wasmBytesRead = 0; wasmBytesRead < wasmDirentBufLen - (2 * wasmDirentSize);) {

            // Make the native syscall. This will read in a list of dirent structs to the buffer
            // We need to read at most two native dirents. Each one will be at most 40 bytes.
            auto nativeBuf = new char[nativeBufLen];
            long nativeBytesRead = syscall(SYS_getdents64, fd, nativeBuf, nativeBufLen);

            if (nativeBytesRead == 0) {
                // No more native dirents
                return wasmBytesRead;
            } else if (nativeBytesRead < 0) {
                // Error reading native dirents
                return (I32) nativeBytesRead;
            }

            // Now we iterate through the dirents we just got back from the host
            int nativeOffset;
            for (nativeOffset = 0; nativeOffset < nativeBytesRead;) {
                // If we're going to overshoot on the wasm buffer, we have a problem
                if (wasmBytesRead > wasmDirentBufLen) {
                    throw std::runtime_error("Overshot the end of the dirent buffer");
                }

                // Get a pointer to the native dirent
                auto d = (struct dirent64 *) (nativeBuf + nativeOffset);

                // Copy the relevant info into the wasm dirent.
                struct wasm_dirent64 dWasm{};
                dWasm.d_ino = (U32) d->d_ino;
                dWasm.d_off = wasmDirentCount * wasmDirentSize;
                dWasm.d_type = d->d_type;
                dWasm.d_reclen = wasmDirentSize;

                // Copy the name into place
                size_t nameLen = strlen(d->d_name);
                std::copy(d->d_name, d->d_name + nameLen, dWasm.d_name);

                // Copy the wasm dirent into place in wasm memory
                auto dWasmBytes = reinterpret_cast<uint8_t *>(&dWasm);
                std::copy(dWasmBytes, dWasmBytes + wasmDirentSize, hostWasmDirentBuf + wasmBytesRead);

                // Move offsets along
                nativeOffset += d->d_reclen;

                wasmBytesRead += wasmDirentSize;
                wasmDirentCount++;
            }
        }

        return wasmBytesRead;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_getdents64", I32, __syscall_getdents64,
                              I32 fd, I32 direntBuf, I32 direntBufSize) {
        return s__syscall_fcntl64(fd, direntBuf, direntBufSize);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall220", I32, ___syscall220, I32 syscallNo, I32 argsPtr) {
        U32 *args = emscriptenArgs(syscallNo, argsPtr, 3);
        return s__syscall_getdents64(args[0], args[1], args[2]);
    }

    I32 s__getcwd(I32 bufPtr, I32 bufLen) {
        util::getLogger()->debug("S - getcwd - {} {}", bufPtr, bufLen);

        // Do nothing here, we want an empty working dir.

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall183", I32, ___syscall183, I32 syscallNo, I32 argsPtr) {
        U32 *args = emscriptenArgs(syscallNo, argsPtr, 2);

        return s__getcwd(args[0], args[1]);
    }


    /**
     * Read is ok provided the function owns the file descriptor
     */
    I32 s__syscall_read(I32 fd, I32 bufPtr, I32 bufLen) {
        util::getLogger()->debug("S - read - {} {} {}", fd, bufPtr, bufLen);

        // Provided the thread owns the fd, we allow reading.
        checkThreadOwnsFd(fd);

        // Get the buffer etc.
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        U8 *buf = Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr) bufPtr, (Uptr) bufLen);

        // Do the actual read
        ssize_t bytesRead = read(fd, buf, (size_t) bufLen);

        return (I32) bytesRead;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_read", I32, __syscall_read, I32 fd, I32 bufPtr, I32 bufLen) {
        return s__syscall_read(fd, bufPtr, bufLen);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall3", I32, ___syscall3, I32 syscallNo, I32 argsPtr) {
        U32 *args = emscriptenArgs(syscallNo, argsPtr, 3);
        return s__syscall_read(args[0], args[1], args[2]);
    }

    /**
    * readlink is ok for certain special cases
    */
    I32 s__syscall_readlink(I32 pathPtr, I32 bufPtr, I32 bufLen) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        char *path = &Runtime::memoryRef<char>(memoryPtr, (Uptr) pathPtr);

        util::getLogger()->debug("S - readlink - {} {} {}", path, bufPtr, bufLen);

        char *buf = Runtime::memoryArrayPtr<char>(memoryPtr, (Uptr) bufPtr, (Uptr) bufLen);

        ssize_t bytesRead = readlink(path, buf, (size_t) bufLen);

        return (I32) bytesRead;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_readlink", I32, __syscall_readlink, I32 pathPtr, I32 bufPtr, I32 count) {
        return s__syscall_readlink(pathPtr, bufPtr, count);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall85", I32, ___syscall85, I32 syscallNo, I32 argsPtr) {
        U32 *args = emscriptenArgs(syscallNo, argsPtr, 3);
        return s__syscall_readlink(args[0], args[1], args[2]);
    }

    I32 s__syscall_close(I32 fd) {
        util::getLogger()->debug("S - close - {}", fd);

        // Provided the thread owns the fd, we allow closing.
        checkThreadOwnsFd(fd);
        openFds.erase(fd);

        close(fd);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_close", I32, __syscall_close, I32 fd) {
        return s__syscall_close(fd);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall6", I32, ___syscall6, I32 syscallNo, I32 argsPtr) {
        U32 *args = emscriptenArgs(syscallNo, argsPtr, 1);
        return s__syscall_close(args[0]);
    }

    /** Poll is ok but can pass in an array of structs. */
    I32 s__syscall_poll(I32 fdsPtr, I32 nfds, I32 timeout) {
        util::getLogger()->debug("S - poll - {} {} {}", fdsPtr, nfds, timeout);

        auto *fds = Runtime::memoryArrayPtr<pollfd>(getExecutingModule()->defaultMemory, (Uptr) fdsPtr, (Uptr) nfds);

        // Check this thread has permission to poll the relevant fds
        for (int i = 0; i < nfds; i++) {
            pollfd fd = fds[i];
            checkThreadOwnsFd(fd.fd);
        }

        int pollRes = poll(fds, (size_t) nfds, timeout);
        return pollRes;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_poll", I32, __syscall_poll, I32 fdsPtr, I32 nfds, I32 timeout) {
        return s__syscall_poll(fdsPtr, nfds, timeout);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall168", I32, ___syscall168, I32 syscallNo, I32 argsPtr) {
        U32 *args = emscriptenArgs(syscallNo, argsPtr, 3);
        return s__syscall_poll(args[0], args[1], args[2]);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "ioctl", I32, ioctl, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - ioctl - {} {} {}", a, b, c);

        return 0;
    }

    I32 s__syscall_ioctl(I32 fd, I32 request, I32 argPtr, I32 d, I32 e, I32 f) {
        util::getLogger()->debug("S - ioctl - {} {} {} {} {} {}", fd, request, argPtr, d, e, f);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_ioctl", I32, __syscall_ioctl,
                              I32 fd, I32 request, I32 argPtr, I32 d, I32 e, I32 f) {
        return s__syscall_ioctl(fd, request, argPtr, d, e, f);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall54", I32, ___syscall54, I32 syscallNo, I32 argsPtr) {
        U32 *args = emscriptenArgs(syscallNo, argsPtr, 6);
        return s__syscall_ioctl(args[0], args[1], args[2], args[3], args[4], args[5]);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "puts", I32, puts, I32 strPtr) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        char *string = &Runtime::memoryRef<char>(memoryPtr, (Uptr) strPtr);

        util::getLogger()->debug("S - puts {}", string);

        return 0;
    }

    /**
     * Writing is ok provided the function owns the file descriptor
     */
    I32 s__syscall_writev(I32 fd, I32 iov, I32 iovcnt) {
        util::getLogger()->debug("S - writev - {} {} {}", fd, iov, iovcnt);
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;

        // Get array of iovecs from memory
        wasm_iovec *iovecs = Runtime::memoryArrayPtr<wasm_iovec>(memoryPtr, iov, iovcnt);

        // Build vector of iovecs
        iovec nativeIovecs[iovcnt];
        for (int i = 0; i < iovcnt; i++) {
            wasm_iovec thisIovec = iovecs[i];

            // Get pointer to data
            U8 *ioData = Runtime::memoryArrayPtr<U8>(memoryPtr, thisIovec.iov_base, thisIovec.iov_len);

            // Create iovec and add to list
            iovec nativeIovec{
                    .iov_base = ioData,
                    .iov_len = thisIovec.iov_len,
            };

            nativeIovecs[i] = nativeIovec;
        }

        Iptr count = writev(STDOUT_FILENO, nativeIovecs, iovcnt);
        fflush(stdout);

        return (I32) count;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_writev", I32, __syscall_writev, I32 fd, I32 iov, I32 iovcnt) {
        return s__syscall_writev(fd, iov, iovcnt);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall146", I32, ___syscall146, I32 syscallNo, I32 argsPtr) {
        U32 *args = emscriptenArgs(syscallNo, argsPtr, 3);

        return s__syscall_writev(args[0], args[1], args[2]);
    }

    /**
     * Writing is ok provided the function owns the file descriptor
     */
    I32 s__syscall_write(I32 fd, I32 bufPtr, I32 bufLen) {
        util::getLogger()->debug("S - write - {} {} {}", fd, bufPtr, bufLen);

        // Provided the thread owns the fd, we allow reading.
        checkThreadOwnsFd(fd);

        // Get the buffer etc.
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        U8 *buf = Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr) bufPtr, (Uptr) bufLen);

        // Do the actual read
        ssize_t bytesWritten = write(fd, buf, (size_t) bufLen);

        return bytesWritten;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_write", I32, __syscall_write, I32 fd, I32 bufPtr, I32 bufLen) {
        return s__syscall_write(fd, bufPtr, bufLen);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall4", I32, ___syscall4, I32 syscallNo, I32 argsPtr) {
        U32 *args = emscriptenArgs(syscallNo, argsPtr, 3);
        return s__syscall_write(args[0], args[1], args[2]);
    }

    I32 s__syscall_mkdir(I32 pathPtr, I32 mode) {
        const std::string fakePath = getMaskedPathFromWasm(pathPtr);

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - mkdir - {} {}", fakePath, mode);

        int res = mkdir(fakePath.c_str(), mode);
        if (res < 0) {
            logger->error("Failed to mkdir at {} - code {}", fakePath, res);
            throw std::runtime_error("Failed on mkdir");
        }

        return res;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall39", I32, ___syscall39, I32 syscallNo, I32 argsPtr) {
        U32 *args = emscriptenArgs(syscallNo, argsPtr, 2);
        return s__syscall_mkdir(args[0], args[1]);
    }

    I32 s__syscall_rename(I32 srcPtr, I32 destPtr) {
        const std::string srcPath = getMaskedPathFromWasm(srcPtr);
        const std::string destPath = getMaskedPathFromWasm(destPtr);

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - rename - {} {}", srcPath, destPath);

        int res = rename(srcPath.c_str(), destPath.c_str());
        if (res != 0) {
            if (res == -EPERM) {
                logger->error("Permission error renaming {} -> {}", srcPath, destPath);
            } else {
                logger->error("Failed renaming {} -> {} - code {}", srcPath, destPath, res);
            }

            throw std::runtime_error("Failed renaming file");
        }
        return res;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall38", I32, ___syscall38, I32 syscallNo, I32 argsPtr) {
        U32 *args = emscriptenArgs(syscallNo, argsPtr, 2);
        return s__syscall_rename(args[0], args[1]);
    }

    I32 s__syscall_unlink(I32 pathPtr) {
        const std::string fakePath = getMaskedPathFromWasm(pathPtr);

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - unlink {}", fakePath);

        int res = unlink(fakePath.c_str());
        if (res < 0) {
            logger->error("Failed to unlink at {} - code {}", fakePath, res);
            throw std::runtime_error("Failed on mkdir");
        }

        return res;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall10", I32, ___syscall10, I32 syscallNo, I32 argsPtr) {
        U32 *args = emscriptenArgs(syscallNo, argsPtr, 1);
        return s__syscall_unlink(args[0]);
    }

    /**
     *  We don't want to give away any real info about the filesystem, but we can just return the default
     *  stat object and catch the application later if it tries to do anything dodgy.
     */
    I32 s__syscall_fstat64(I32 fd, I32 statBufPtr) {
        util::getLogger()->debug("S - fstat64 - {} {}", fd, statBufPtr);

        struct stat64 nativeStat{};
        fstat64(fd, &nativeStat);
        writeNativeStatToWasmStat(&nativeStat, statBufPtr);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_fstat64", I32, __syscall_fstat64, I32 fd, I32 statBufPtr) {
        return s__syscall_fstat64(fd, statBufPtr);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall197", I32, ___syscall197, I32 syscallNo, I32 argsPtr) {
        U32 *args = emscriptenArgs(syscallNo, argsPtr, 2);

        return s__syscall_fstat64(args[0], args[1]);
    }

    I32 s__syscall_stat64(I32 pathPtr, I32 statBufPtr) {
        // Get the path
        const std::string fakePath = getMaskedPathFromWasm(pathPtr);

        util::getLogger()->debug("S - stat64 - {} {}", fakePath, statBufPtr);

        struct stat64 nativeStat{};
        stat64(fakePath.c_str(), &nativeStat);
        writeNativeStatToWasmStat(&nativeStat, statBufPtr);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_stat64", I32, __syscall_stat64, I32 pathPtr, I32 statBufPtr) {
        return s__syscall_stat64(pathPtr, statBufPtr);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall195", I32, ___syscall195, I32 syscallNo, I32 argsPtr) {
        U32 *args = emscriptenArgs(syscallNo, argsPtr, 2);

        return s__syscall_stat64(args[0], args[1]);
    }

    I32 s__syscall_lstat64(I32 pathPtr, I32 statBufPtr) {
        const std::string fakePath = getMaskedPathFromWasm(pathPtr);
        util::getLogger()->debug("S - lstat - {} {}", fakePath, statBufPtr);

        struct stat64 nativeStat{};
        lstat64(fakePath.c_str(), &nativeStat);
        writeNativeStatToWasmStat(&nativeStat, statBufPtr);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall196", I32, ___syscall196, I32 syscallNo, I32 argsPtr) {
        U32 *args = emscriptenArgs(syscallNo, argsPtr, 2);

        return s__syscall_lstat64(args[0], args[1]);
    }

    // ------------------------
    // I/O - unsupported
    // ------------------------

    I32 s__syscall_readv(I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - readv - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_readv", I32, __syscall_readv, I32 a, I32 b, I32 c) {
        return s__syscall_readv(a, b, c);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall145", I32, ___syscall145, I32 syscallNo, I32 argsPtr) {
        U32 *args = emscriptenArgs(syscallNo, argsPtr, 3);
        return s__syscall_readv(args[0], args[1], args[2]);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___map_file", I32, ___map_file, I32 a, I32 b) {
        util::getLogger()->debug("S - ___map_file - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    /**
     * Although llseek is being called, musl is using it within the implementation of lseek,
     * therefore we can just use lseek as a shortcut
     */
    I32 s__syscall_llseek(I32 fd, I32 offsetHigh, I32 offsetLow, I32 resultPtr, I32 whence) {
        util::getLogger()->debug("S - llseek - {} {} {} {} {}", fd, offsetHigh, offsetLow, resultPtr, whence);

        checkThreadOwnsFd(fd);

        int res = (int) lseek(fd, offsetLow, whence);

        return (I32) res;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_llseek", I32, __syscall_llseek, I32 a, I32 b, I32 c, I32 d, I32 e) {
        return s__syscall_llseek(a, b, c, d, e);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall140", I32, ___syscall140, I32 syscallNo, I32 argsPtr) {
        U32 *args = emscriptenArgs(syscallNo, argsPtr, 5);

        return s__syscall_llseek(args[0], args[1], args[2], args[3], args[4]);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_futex", I32, __syscall_futex,
                              I32 uaddr, I32 futexOp, I32 val, I32 timeoutPtr, I32 uaddr2, I32 val2) {
        util::getLogger()->debug("S - futex - {} {} {} {} {} {}", uaddr, futexOp, val, timeoutPtr, uaddr2, val2);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_access", I32, __syscall_access, I32 a, I32 b) {
        util::getLogger()->debug("S - access - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_chown32", I32, __syscall_chown32, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - chown32 - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_openat", I32, __syscall_openat, I32 a, I32 b, I32 c, I32 d) {
        util::getLogger()->debug("S - openat - {} {} {} {}", a, b, c, d);

        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_fsync", I32, __syscall_fsync, I32 a) {
        util::getLogger()->debug("S - fsync - {}", a);

        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_dup2", I32, __syscall_dup2, I32 a, I32 b) {
        util::getLogger()->debug("S - dup2 - {} {}", a, b);

        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_pipe", I32, __syscall_pipe, I32 a) {
        util::getLogger()->debug("S - pipe - {}", a);

        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall__newselect", I32, __syscall__newselect, I32 a, I32 b, I32 c, I32 d,
                              I32 e) {
        util::getLogger()->debug("S - newselect - {} {} {} {} {}", a, b, c, d, e);

        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_fdatasync", I32, __syscall_fdatasync, I32 a) {
        util::getLogger()->debug("S - fdatasync - {}", a);

        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    // ------------------------
    // Filesystem - unsupported
    // ------------------------

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_creat", I32, __syscall_creat, I32 a, I32 b) {
        util::getLogger()->debug("S - creat - {} {}", a, b);

        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_unlink", I32, __syscall_unlink, I32 a) {
        util::getLogger()->debug("S - unlink - {}", a);

        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_chdir", I32, __syscall_chdir, I32 a) {
        util::getLogger()->debug("S - chdir - {}", a);

        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_umask", I32, __syscall_umask, I32 a) {
        util::getLogger()->debug("S - umask - {}", a);

        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_lstat64", I32, __syscall_lstat64, I32 a, I32 b) {
        util::getLogger()->debug("S - lstat64 - {} {}", a, b);

        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_rename", I32, __syscall_rename, I32 a, I32 b) {
        util::getLogger()->debug("S - rename - {} {}", a, b);

        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    // ------------------------
    // Sockets/ network
    // ------------------------

    enum SocketCalls : U32 {
        sc_socket = 1,
        sc_bind,
        sc_connect,
        sc_listen,
        sc_accept,
        sc_getsockname,
        sc_getpeername,
        sc_socketpair,
        sc_send,
        sc_recv,
        sc_sendto,
        sc_recvfrom,
        sc_shutdown,
        sc_setsockopt,
        sc_getsockopt,
        sc_sendmsg,
        sc_recvmsg,
        sc_accept4,
        sc_recvmmsg,
        sc_sendmmsg,
    };

    /** Writes changes to a native sockaddr back to a wasm sockaddr. This is important in several
     * networking syscalls that receive responses and modify arguments in place */
    void setSockAddr(sockaddr nativeSockAddr, I32 addrPtr) {
        // Get pointer to wasm address
        wasm_sockaddr *wasmAddrPtr = &Runtime::memoryRef<wasm_sockaddr>(getExecutingModule()->defaultMemory,
                                                                        (Uptr) addrPtr);

        // Modify in place
        wasmAddrPtr->sa_family = nativeSockAddr.sa_family;
        std::copy(nativeSockAddr.sa_data, nativeSockAddr.sa_data + 14, wasmAddrPtr->sa_data);
    }

    void setSockLen(socklen_t nativeValue, I32 wasmPtr) {
        // Get pointer to wasm address
        I32 *wasmAddrPtr = &Runtime::memoryRef<I32>(getExecutingModule()->defaultMemory, (Uptr) wasmPtr);
        std::copy(&nativeValue, &nativeValue + 1, wasmAddrPtr);
    }

    /**
     * When properly isolated, functions will run in their own network namespace,
     * therefore we can be relatively comfortable passing some of the syscalls
     * straight through.
     */
    I32 s__syscall_socketcall(I32 call, I32 argsPtr) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;

        // NOTE
        // We don't want to support server-side socket syscalls as we expect functions
        // only to be clients

        switch (call) {
            // ----------------------------
            // Suppported
            // ----------------------------

            case (SocketCalls::sc_socket): {
                // Socket constants:
                // https://github.com/Shillaker/musl/blob/wasm-prototype-1/include/sys/socket.h
                // https://github.com/Shillaker/musl/blob/wasm-prototype-1/include/netinet/in.h

                U32 *subCallArgs = Runtime::memoryArrayPtr<U32>(memoryPtr, argsPtr, 3);
                U32 domain = subCallArgs[0];
                U32 type = subCallArgs[1];
                U32 protocol = subCallArgs[2];

                switch (domain) {
                    case (2): {
                        domain = AF_INET;
                        break;
                    }
                    case (10): {
                        domain = AF_INET6;
                        break;
                    }
                    default: {
                        printf("Unrecognised domain (%u)\n", domain);
                        break;
                    }
                }

                switch (type) {
                    case (1): {
                        type = SOCK_STREAM;
                        break;
                    }
                    case (2): {
                        type = SOCK_DGRAM;
                        break;
                    }
                    default: {
                        printf("Unrecognised socket type (%u)\n", type);
                        break;
                    }
                }

                switch (protocol) {
                    case (0): {
                        protocol = IPPROTO_IP;
                        break;
                    }
                    case (6): {
                        protocol = IPPROTO_TCP;
                        break;
                    }
                    default: {
                        printf("Unrecognised protocol (%u)\n", protocol);
                        break;
                    }
                }

                util::getLogger()->debug("S - socket - {} {} {}", domain, type, protocol);
                I32 sock = (int) syscall(SYS_socket, domain, type, protocol);

                if (sock < 0) {
                    printf("Socket error: %i\n", sock);
                }

                openFds.insert(sock);

                return sock;
            }

            case (SocketCalls::sc_connect): {
                U32 *subCallArgs = Runtime::memoryArrayPtr<U32>(memoryPtr, argsPtr, 3);

                I32 sockfd = subCallArgs[0];
                I32 addrPtr = subCallArgs[1];
                I32 addrLen = subCallArgs[2];

                util::getLogger()->debug("S - connect - {} {} {}", sockfd, addrPtr, addrLen);

                // Allow connecting if thread owns socket
                checkThreadOwnsFd(sockfd);

                sockaddr addr = getSockAddr(addrPtr);
                int result = connect(sockfd, &addr, sizeof(sockaddr));

                return result;
            }

            case (SocketCalls::sc_recv):
            case (SocketCalls::sc_recvfrom):
            case (SocketCalls::sc_sendto):
            case (SocketCalls::sc_send): {
                int argCount = 4;

                if (call == SocketCalls::sc_sendto || call == SocketCalls::sc_recvfrom) {
                    argCount = 6;
                }

                // Pull out arguments
                U32 *subCallArgs = Runtime::memoryArrayPtr<U32>(memoryPtr, (Uptr) argsPtr, (Uptr) argCount);
                I32 sockfd = subCallArgs[0];
                Uptr bufPtr = subCallArgs[1];
                size_t bufLen = subCallArgs[2];
                I32 flags = subCallArgs[3];

                // Make sure thread owns this socket
                checkThreadOwnsFd(sockfd);

                // Set up buffer
                U8 *buf = Runtime::memoryArrayPtr<U8>(memoryPtr, bufPtr, bufLen);

                ssize_t result = 0;
                if (call == SocketCalls::sc_send) {
                    util::getLogger()->debug("S - send - {} {} {} {}", sockfd, bufPtr, bufLen, flags);

                    result = send(sockfd, buf, bufLen, flags);

                } else if (call == SocketCalls::sc_recv) {
                    util::getLogger()->debug("S - recv - {} {} {} {}", sockfd, bufPtr, bufLen, flags);

                    result = recv(sockfd, buf, bufLen, flags);

                } else {
                    I32 sockAddrPtr = subCallArgs[4];
                    sockaddr sockAddr = getSockAddr(sockAddrPtr);
                    socklen_t nativeAddrLen = sizeof(sockAddr);
                    socklen_t addrLen = subCallArgs[5];

                    if (call == SocketCalls::sc_sendto) {
                        util::getLogger()->debug("S - sendto - {} {} {} {} {} {}", sockfd, bufPtr, bufLen, flags,
                                                 sockAddrPtr,
                                                 addrLen);

                        result = sendto(sockfd, buf, bufLen, flags, &sockAddr, nativeAddrLen);

                    } else {
                        // Note, addrLen here is actually a pointer
                        util::getLogger()->debug("S - recvfrom - {} {} {} {} {} {}", sockfd, bufPtr, bufLen, flags,
                                                 sockAddrPtr,
                                                 addrLen);

                        // Make the call
                        result = recvfrom(sockfd, buf, bufLen, flags, &sockAddr, &nativeAddrLen);

                        // Note, recvfrom will modify the sockaddr and addrlen in place with the details returned
                        // from the host, therefore we must also modify the original wasm object
                        setSockAddr(sockAddr, sockAddrPtr);
                        setSockLen(nativeAddrLen, addrLen);
                    }
                }

                return (I32) result;
            }

            case (SocketCalls::sc_bind): {
                U32 *subCallArgs = Runtime::memoryArrayPtr<U32>(memoryPtr, argsPtr, 3);
                I32 sockfd = subCallArgs[0];

                I32 addrPtr = subCallArgs[1];
                sockaddr addr = getSockAddr(addrPtr);

                I32 addrLen = subCallArgs[2];

                util::getLogger()->debug("S - bind - {} {} {}", sockfd, addrPtr, addrLen);

                // If thread owns fd, we can bind
                checkThreadOwnsFd(sockfd);
                int bindResult = bind(sockfd, &addr, sizeof(addr));

                return (I32) bindResult;
            }

            case (SocketCalls::sc_getsockname): {
                U32 *subCallArgs = Runtime::memoryArrayPtr<U32>(memoryPtr, argsPtr, 3);
                I32 sockfd = subCallArgs[0];
                I32 addrPtr = subCallArgs[1];
                I32 addrLenPtr = subCallArgs[2];

                util::getLogger()->debug("S - getsockname - {} {} {}", sockfd, addrPtr, addrLenPtr);

                checkThreadOwnsFd(sockfd);

                sockaddr nativeAddr = getSockAddr(addrPtr);
                socklen_t nativeAddrLen = sizeof(nativeAddr);

                int result = getsockname(sockfd, &nativeAddr, &nativeAddrLen);

                // Make sure we write any results back to the wasm objects
                setSockAddr(nativeAddr, addrPtr);
                setSockLen(nativeAddrLen, addrLenPtr);

                return result;
            }

                // ----------------------------
                // Unfinished
                // ----------------------------

            case (SocketCalls::sc_getpeername): {
                util::getLogger()->debug("S - getpeername - {} {}", call, argsPtr);
                return 0;
            }

            case (SocketCalls::sc_socketpair): {
                util::getLogger()->debug("S - socketpair - {} {}", call, argsPtr);
                return 0;
            }

            case (SocketCalls::sc_shutdown): {
                util::getLogger()->debug("S - shutdown - {} {}", call, argsPtr);
                return 0;
            }

            case (SocketCalls::sc_setsockopt): {
                util::getLogger()->debug("S - setsockopt - {} {}", call, argsPtr);
                return 0;
            }
            case (SocketCalls::sc_getsockopt): {
                util::getLogger()->debug("S - getsockopt - {} {}", call, argsPtr);
                return 0;
            }

            case (SocketCalls::sc_sendmsg): {
                util::getLogger()->debug("S - sendmsg - {} {}", call, argsPtr);
                return 0;
            }

            case (SocketCalls::sc_recvmsg): {
                util::getLogger()->debug("S - recvmsg - {} {}", call, argsPtr);
                return 0;
            }

            case (SocketCalls::sc_accept4): {
                util::getLogger()->debug("S - accept4 - {} {}", call, argsPtr);
                return 0;
            }

            case (SocketCalls::sc_recvmmsg): {
                util::getLogger()->debug("S - recvmmsg - {} {}", call, argsPtr);
                return 0;
            }

            case (SocketCalls::sc_sendmmsg): {
                util::getLogger()->debug("S - sendmmsg - {} {}", call, argsPtr);
                return 0;
            }

                // ----------------------------
                // Not supported
                // ----------------------------

            case (SocketCalls::sc_accept):
                // Server-side
                util::getLogger()->debug("S - accept - {} {}", call, argsPtr);

                throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);

            case (SocketCalls::sc_listen): {
                // Server-side
                util::getLogger()->debug("S - listen - {} {}", call, argsPtr);

                throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
            }

            default: {
                printf("Unrecognised socketcall %i\n", call);
                return 0;
            }
        }

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_socketcall", I32, __syscall_socketcall, I32 call, I32 argsPtr) {
        return s__syscall_socketcall(call, argsPtr);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall102", I32, ___syscall102, I32 syscallNo, I32 argsPtr) {
        U32 *args = emscriptenArgs(syscallNo, argsPtr, 2);
        return s__syscall_socketcall(args[0], args[1]);
    }

    I32 s__gethostbyname(I32 hostnamePtr) {
        const std::string hostname = getStringFromWasm(hostnamePtr);
        util::getLogger()->debug("S - gethostbyname {}", hostname);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "_gethostbyname", I32, _gethostbyname, I32 hostnamePtr) {
        return s__gethostbyname(hostnamePtr);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_gethostbyname", I32, emscripten_gethostbyname, I32 hostnamePtr) {
        return s__gethostbyname(hostnamePtr);
    }

    // ------------------------
    // Timing - supported
    // ------------------------

    //TODO - make timing functions more secure
    I32 s__syscall_clock_gettime(I32 clockId, I32 resultAddress) {
        util::getLogger()->debug("S - clock_gettime - {} {}", clockId, resultAddress);

        timespec ts{};
        clock_gettime(CLOCK_MONOTONIC, &ts);

        auto result = &Runtime::memoryRef<wasm_timespec>(getExecutingModule()->defaultMemory, (Uptr) resultAddress);
        result->tv_sec = I32(ts.tv_sec);
        result->tv_nsec = I32(ts.tv_nsec);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_clock_gettime", I32, __syscall_clock_gettime,
                              I32 clockId, I32 resultAddress) {
        return s__syscall_clock_gettime(clockId, resultAddress);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_clock_gettime", I32, _clock_gettime, I32 syscallNo, I32 argsPtr) {
        U32 *args = emscriptenArgs(syscallNo, argsPtr, 2);

        return s__syscall_clock_gettime(args[0], args[1]);
    }

    /**
     * As specified in the gettimeofday man page, use of the timezone struct is obsolete and hence not supported here
     */
    I32 s__syscall_gettimeofday(I32 timevalPtr, I32 tzPtr) {
        util::getLogger()->debug("S - gettimeofday - {} {}", timevalPtr, tzPtr);

        timeval tv{};
        gettimeofday(&tv, nullptr);

        auto result = &Runtime::memoryRef<wasm_timeval>(getExecutingModule()->defaultMemory, (Uptr) timevalPtr);
        result->tv_sec = I32(tv.tv_sec);
        result->tv_usec = I32(tv.tv_usec);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_gettimeofday", I32, __syscall_gettimeofday, I32 timevalPtr, I32 tzPtr) {
        return s__syscall_gettimeofday(timevalPtr, tzPtr);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_gettimeofday", I32, _gettimeofday, I32 syscallNo, I32 argsPtr) {
        U32 *args = emscriptenArgs(syscallNo, argsPtr, 2);

        return s__syscall_gettimeofday(args[0], args[1]);
    }

    // ------------------------
    // Timing - unsupported
    // ------------------------

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_setitimer", I32, __syscall_setitimer, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - setitimer - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_clock", I32, _clock) {
        util::getLogger()->debug("S - _clock");
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_clock_getres", I32, _clock_getres, I32 a, I32 b) {
        util::getLogger()->debug("S - _clock_getres - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_localtime", I32, _localtime, I32 a) {
        util::getLogger()->debug("S - _localtime - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_times", I32, _times, I32 a) {
        util::getLogger()->debug("S - _times - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_mktime", I32, _mktime, I32 a) {
        util::getLogger()->debug("S - _mktime - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_wait4", I32, __syscall_wait4, I32 a, I32 b, I32 c, I32 d) {
        util::getLogger()->debug("S - wait4 - {} {} {} {}", a, b, c, d);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    // ------------------------
    // Program control - supported
    // ------------------------

    DEFINE_INTRINSIC_FUNCTION(env, "exit", void, exit, I32 a) {
        util::getLogger()->debug("S - exit - {}", a);
        throw (wasm::WasmExitException(a));
    }

    DEFINE_INTRINSIC_FUNCTION(env, "_Exit", void, _Exit, I32 a) {
        util::getLogger()->debug("S - _Exit - {}", a);
        throw (wasm::WasmExitException(a));
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_exit", I32, __syscall_exit, I32 a) {
        util::getLogger()->debug("S - exit - {}", a);
        throw (wasm::WasmExitException(a));
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_exit_group", I32, __syscall_exit_group, I32 a) {
        util::getLogger()->debug("S - exit_group - {}", a);
        throw (wasm::WasmExitException(a));
    }

    // ------------------------
    // Program control - unsupported
    // ------------------------

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_raise", I32, _raise, I32 a) {
        util::getLogger()->debug("S - _raise - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_waitpid", I32, _waitpid, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - _waitpid - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_fork", I32, _fork) {
        util::getLogger()->debug("S - _fork");
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    // ------------------------
    // Signals
    // ------------------------

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_sigaction", I32, _sigaction, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - sigaction - {} {} {}", a, b, c);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_sigemptyset", I32, _sigemptyset, I32 a) {
        util::getLogger()->debug("S - _sigemptyset - {}", a);

        return 0;
    }

    // ------------------------
    // Threads - unsupported
    // ------------------------
    DEFINE_INTRINSIC_FUNCTION(emEnv, "_pthread_setcancelstate", I32, _pthread_setcancelstate, I32 a, I32 b) {
        util::getLogger()->debug("S - _pthread_setcancelstate - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_pthread_sigmask", I32, _pthread_sigmask, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - _pthread_sigmask - {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    // ------------------------
    // Misc
    // ------------------------

    I32 s__getpwuid(I32 uid) {
        util::getLogger()->debug("S - _getpwuid - {}", uid);

        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;

        // Provision space for the new struct, username and password
        U32 namePtr = dynamicAllocString(memoryPtr, FAKE_NAME, strlen(FAKE_NAME));
        U32 passwordPtr = dynamicAllocString(memoryPtr, FAKE_PASSWORD, strlen(FAKE_PASSWORD));
        U32 homePtr = dynamicAllocString(memoryPtr, FAKE_HOME, strlen(FAKE_HOME));
        U32 structPtr = dynamicAlloc(memoryPtr, sizeof(wasm_passwd));

        // Get a pointer to it
        wasm_passwd *wasmPasswd = &Runtime::memoryRef<wasm_passwd>(memoryPtr, (Uptr) structPtr);

        wasmPasswd->pw_name = namePtr;
        wasmPasswd->pw_passwd = passwordPtr;
        wasmPasswd->pw_uid = FAKE_UID;
        wasmPasswd->pw_gid = FAKE_GID;
        wasmPasswd->pw_dir = homePtr;

        return structPtr;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_getpwuid", I32, _getpwuid, I32 uid) {
        return s__getpwuid(uid);
    }

    I32 s__getuid32() {
        util::getLogger()->debug("S - getuid32");
        return FAKE_UID;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall199", I32, ___syscall199, I32 syscallNo, I32 argsPtr) {
        return s__getuid32();
    }

    I32 s__getgid32() {
        util::getLogger()->debug("S - getgid32");
        return FAKE_GID;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall200", I32, ___syscall200, I32 syscallNo, I32 argsPtr) {
        return s__getgid32();
    }

    I32 s__geteuid32() {
        util::getLogger()->debug("S - geteuid32");
        // We will fake the process running as uid 1
        return FAKE_UID;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall201", I32, ___syscall201, I32 syscallNo, I32 argsPtr) {
        return s__geteuid32();
    }

    I32 s__getegid32() {
        util::getLogger()->debug("S - getegid32");

        return FAKE_GID;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall202", I32, ___syscall202, I32 syscallNo, I32 argsPtr) {
        return s__getegid32();
    }

    U32 s__getenv(I32 varPtr, bool isEmscripten) {
        WasmModule *module = getExecutingModule();
        Runtime::Memory *memoryPtr = module->defaultMemory;
        char *varName = &Runtime::memoryRef<char>(memoryPtr, (Uptr) varPtr);

        util::SystemConfig &conf = util::getSystemConfig();

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - _getenv - {}", varName);

        const char *value = "";

        // Special cases
        if (strcmp(varName, "PYTHONHASHSEED") == 0) {
            value = "0";
        } else if (strcmp(varName, "PYTHONVERBOSE") == 0) {
            value = "on";
        } else if (strcmp(varName, "PYTHONHOME") == 0) {
            value = "/";
        } else if (strcmp(varName, "PYTHONPATH") == 0) {
            value = "/";
        } else if (strcmp(varName, "LC_CTYPE") == 0) {
            value = "en_GB.UTF-8";
//        } else if (strcmp(varName, "LANG") == 0) {
//            value = "en_GB.UTF-8";
//        } else if (strcmp(varName, "LANGUAGE") == 0) {
//            value = "en_GB:en";
        } else if (strcmp(varName, "FULL_ASYNC") == 0) {
            if (conf.fullAsync == 1) {
                value = "1";
            } else {
                value = "0";
            }
        } else if (strcmp(varName, "FULL_SYNC") == 0) {
            if (conf.fullSync == 1) {
                value = "1";
            } else {
                value = "0";
            }
        }

        U32 result;
        size_t nBytes = strlen(value) + 1;
        if (isEmscripten) {
            result = dynamicAllocString(memoryPtr, value, nBytes);
        } else {
            // TODO - avoid allocating a whole page just for a new string
            result = module->mmap(nBytes);
            char *hostAddr = Runtime::memoryArrayPtr<char>(memoryPtr, result, nBytes);
            std::copy(value, value + nBytes, hostAddr);
            logger->debug("Copied {}={} ({} bytes)", varName, value, nBytes);
        }

        return result;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "_getenv", I32, _getenv, I32 varPtr) {
        return s__getenv(varPtr, false);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_getenv", I32, _emscriptenGetenv, I32 varPtr) {
        return s__getenv(varPtr, true);
    }

    I32 s__setenv(I32 varPtr, I32 valPtr, I32 overwrite) {
        const std::string varName = getStringFromWasm(varPtr);
        const std::string value = getStringFromWasm(valPtr);

        util::getLogger()->debug("S - _setenv {} {} {}", varName, value, overwrite);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "_setenv", I32, _setenv, I32 varPtr, I32 valPtr, I32 overwrite) {
        return s__setenv(varPtr, valPtr, overwrite);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_setenv", I32, emscripten_setenv, I32 varPtr, I32 valPtr, I32 overwrite) {
        return s__setenv(varPtr, valPtr, overwrite);
    }

    I32 s__unsetenv(I32 varPtr) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        char *varName = &Runtime::memoryRef<char>(memoryPtr, (Uptr) varPtr);

        util::getLogger()->debug("S - _unsetenv {} {} {}", varName);

        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "_unsetenv", I32, _unsetenv, I32 varPtr) {
        return s__unsetenv(varPtr);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_unsetenv", I32, _emscriptenUnsetenv, I32 varPtr) {
        return s__unsetenv(varPtr);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "abort", void, abort) {
        util::getLogger()->debug("S - abort");
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "__exit", void, __exit, I32 a) {
        util::getLogger()->debug("S - __exit - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_rt_sigaction", I32, __syscall_rt_sigaction, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - rt_sigaction - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__funcs_on_exit", void, __funcs_on_exit) {
        util::getLogger()->debug("S - funcs_on_exit", "");
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__libc_exit_fini", void, __libc_exit_fini) {
        util::getLogger()->debug("S - libc_exit_fini", "");
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__stdio_exit", void, __stdio_exit) {
        util::getLogger()->debug("S - stdio_exit", "");
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__unsupported_syscall", I32, __unsupported_syscall,
                              I32 a, I32 b, I32 c, I32 d, I32 e, I32 f, I32 g) {
        util::getLogger()->debug("S - UNSUPPORTED - {} {} {} {} {} {} {}", a, b, c, d, e, f, g);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_gettid", I32, __syscall_gettid, I32 a) {
        util::getLogger()->debug("S - gettid - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_getgroups32", I32, __syscall_getgroups32, I32 a, I32 b) {
        util::getLogger()->debug("S - getgroups32 - {} {}", a, b);

        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_tkill", I32, __syscall_tkill, I32 a, I32 b) {
        util::getLogger()->debug("S - tkill - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_kill", I32, __syscall_kill, I32 a, I32 b) {
        util::getLogger()->debug("S - kill - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_tgkill", I32, __syscall_tgkill, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - tgkill - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_sched_getaffinity", I32, __syscall_sched_getaffinity, I32 a, I32 b,
                              I32 c) {
        util::getLogger()->debug("S - sched_getaffinity - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_faccessat", I32, __syscall_faccessat, I32 a, I32 b, I32 c, I32 d) {
        util::getLogger()->debug("S - sched_faccessat - {} {} {} {}", a, b, c, d);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_pselect6", I32, __syscall_pselect6, I32 a, I32 b, I32 c, I32 d, I32 e,
                              I32 f) {
        util::getLogger()->debug("S - pselect6 - {} {} {} {} {} {}", a, b, c, d, e, f);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_rt_sigprocmask", I32, __syscall_rt_sigprocmask, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - rt_sigprocmask - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "_Unwind_RaiseException", I32, _Unwind_RaiseException, I32 a) {
        util::getLogger()->debug("S - Unwind_RaiseException - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "_Unwind_DeleteException", void, _Unwind_DeleteException, I32 a) {
        util::getLogger()->debug("S - Unwind_DeleteException - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_getpwent", I32, _getpwent) {
        util::getLogger()->debug("S - _getpwent");
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_endpwent", void, _endpwent) {
        util::getLogger()->debug("S - _endpwent");
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_fpathconf", I32, _fpathconf, I32 a, I32 b) {
        util::getLogger()->debug("S - _fpathconf - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    // ------------------------
    // LLVM/ Emscripten
    // ------------------------

    /**
     * It appears we can ignore this llvm_stacksave and llvm_stackrestore although I'm
     * not 100% sure when/ why they get called.
     */
    DEFINE_INTRINSIC_FUNCTION(emEnv, "_llvm_stacksave", I32, _llvm_stacksave) {
        util::getLogger()->debug("S - llvm_stacksave");

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_llvm_stackrestore", void, _llvm_stackrestore, I32 a) {
        util::getLogger()->debug("S - llvm_stackrestore");
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_llvm_log2_f64", F64, _llvm_log2_f64, F64 a) {
        util::getLogger()->debug("S - _llvm_log2_f64");
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_llvm_log10_f64", F64, _llvm_log10_f64, F64 a) {
        util::getLogger()->debug("S - _llvm_log10_f64");
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    // ------------------------
    // Memory - supported
    // ------------------------

    /**
     * We can permit mmap as a means to grow memory via anonymous mappings. As there is only one
     * address range, this may end up conflicting with other memory management for the module.
     */
    I32 s__syscall_mmap(U32 addr, U32 length, U32 prot, U32 flags, I32 fd, U32 offset) {
        util::getLogger()->debug("S - mmap - {} {} {} {} {} {}", addr, length, prot, flags, fd, offset);

        // Note - we are ignoring the offset input

        if (addr != 0) {
            printf("Ignoring mmap hint at %i\n", addr);
        }

        // fd != -1 is non-anonymous mapping
        if (fd != -1) {
            throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
        }

        WasmModule *module = getExecutingModule();

        return module->mmap(length);
    }


    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_mmap", I32, __syscall_mmap,
                              U32 addr, U32 length, U32 prot, U32 flags, I32 fd, U32 offset) {
        return s__syscall_mmap(addr, length, prot, flags, fd, offset);
    }

    /**
     * syscall 192 is mmap2, which has the same interface as mmap except that the final argument specifies
     * the offset into the file in 4096-byte units (instead of bytes, as is done by mmap). Given that we
     * ignore the offset we can just treat it like mmap
     */
    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall192", I32, ___syscall192, I32 syscallNo, I32 argsPtr) {
        U32 *args = emscriptenArgs(syscallNo, argsPtr, 6);

        return s__syscall_mmap(args[0], args[1], args[2], args[3], args[4], args[5]);
    }

    /**
     * munmap is allowed to shrink the available module memory
     */
    I32 s__syscall_munmap(U32 addr, U32 length) {
        util::getLogger()->debug("S - munmap - {} {}", addr, length);

        Runtime::Memory *memory = getExecutingModule()->defaultMemory;

        // If not aligned or zero length, drop out
        if (addr & (IR::numBytesPerPage - 1) || length == 0) {
            return -EINVAL;
        }

        const Uptr basePageIndex = addr / IR::numBytesPerPage;
        const Uptr numPages = getNumberOfPagesForBytes(length);

        // Drop out if we're munmapping over the max page boundary
        if (basePageIndex + numPages > getMemoryMaxPages(memory)) {
            return -EINVAL;
        }

        // If these are the top pages of memory, shrink it, if not, unmap them
        // Note that we won't be able to reclaim them if we're just unmapping
        const Uptr currentPageCount = getMemoryNumPages(memory);
        if (basePageIndex + numPages == currentPageCount) {
            shrinkMemory(memory, numPages);
        } else {
            unmapMemoryPages(memory, basePageIndex, numPages);
        }

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_munmap", I32, __syscall_munmap, U32 addr, U32 length) {
        return s__syscall_munmap(addr, length);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall91", I32, ___syscall91, U32 syscallNo, U32 argsPtr) {
        U32 *args = emscriptenArgs(syscallNo, argsPtr, 2);
        return s__syscall_munmap(args[0], args[1]);
    }

    /**
   * brk should be fine to run in most cases, need to check limits on the process' memory.
   *
   * Details of the return value aren't particularly clear. I took info from https://linux.die.net/man/2/brk
   * i.e.
   *
   *   - on addr == 0 return current break
   *   - on error return current break
   *   - on success return NEW break
   */

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_brk", I32, __syscall_brk, U32 addr) {
        util::getLogger()->debug("S - brk - {}", addr);

        Runtime::Memory *memory = getExecutingModule()->defaultMemory;
        const U32 currentBreak = (U32) ((getMemoryNumPages(memory) * IR::numBytesPerPage));

        // Attempt the expansion
        int expandRes = expandMemory(addr);

        if (expandRes == EXPAND_TOO_BIG) {
            return currentBreak;
        } else if (expandRes == EXPAND_NO_ACTION) {
            return currentBreak;
        } else {
            // Success, return requested break
            return (U32) addr;
        }
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_madvise", I32, __syscall_madvise,
                              U32 address, U32 numBytes, U32 advice) {
        util::getLogger()->debug("S - madvise - {} {} {}", address, numBytes, advice);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_membarrier", I32, __syscall_membarrier, I32 a, I32 b) {
        util::getLogger()->debug("S - membarrier - {} {}", a, b);

        return 0;
    }

    // ------------------------
    // Memory - unsupported
    // ------------------------

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_mremap", I32, __syscall_mremap,
                              U32 oldAddress, U32 oldNumBytes, U32 newNumBytes, U32 flags, U32 newAddress) {
        util::getLogger()->debug("S - mremap - {} {} {} {} {}", oldAddress, oldNumBytes, newNumBytes, flags,
                                 newAddress);

        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    // ------------------------
    // Resources - unsupported
    // ------------------------

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_setrlimit", I32, __syscall_setrlimit, I32 a, I32 b) {
        util::getLogger()->debug("S - setrlimit - {} {}", a, b);

        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_getrlimit", I32, __syscall_getrlimit, I32 a, I32 b) {
        util::getLogger()->debug("S - getrlimit - {} {}", a, b);

        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_getrusage", I32, __syscall_getrusage, I32 a, I32 b) {
        util::getLogger()->debug("S - getrusage - {} {}", a, b);

        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_ugetrlimit", I32, __syscall_ugetrlimit, I32 a, I32 b) {
        util::getLogger()->debug("S - ugetrlimit - {} {}", a, b);

        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_prlimit64", I32, __syscall_prlimit64, I32 a, I32 b, I32 c, I32 d) {
        util::getLogger()->debug("S - prlimit64 - {} {} {} {}", a, b, c, d);

        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }


    /**
     * FROM HERE IS COPY-PASTED FROM THE WAVM Emscripten.cpp file
     *
     * TODO - USE THE ACTUAL FILE
     */

    DEFINE_INTRINSIC_FUNCTION(emAsm2wasm, "i32u-rem", U32, I32_remu, U32 left, U32 right) {
        return left % right;
    }

    DEFINE_INTRINSIC_FUNCTION(emAsm2wasm, "i32s-rem", I32, I32_rems, I32 left, I32 right) {
        return left % right;
    }

    DEFINE_INTRINSIC_FUNCTION(emAsm2wasm, "i32u-div", U32, I32_divu, U32 left, U32 right) {
        return left / right;
    }

    DEFINE_INTRINSIC_FUNCTION(emAsm2wasm, "i32s-div", I32, I32_divs, I32 left, I32 right) {
        return left / right;
    }

    DEFINE_INTRINSIC_FUNCTION(emAsm2wasm, "f64-to-int", I32, f64_to_int, F64 f) {
        return (I32) f;
    }

    DEFINE_INTRINSIC_FUNCTION(emAsm2wasm, "f64-rem", F64, F64_rems, F64 left, F64 right) {
        return (F64) fmod(left, right);
    }

    DEFINE_INTRINSIC_GLOBAL(emEnv, "STACKTOP", I32, STACKTOP, EMSCRIPTEN_STACKTOP);
    DEFINE_INTRINSIC_GLOBAL(emEnv, "STACK_MAX", I32, STACK_MAX, EMSCRIPTEN_STACK_MAX);
    DEFINE_INTRINSIC_GLOBAL(emEnv,
                            "tempDoublePtr",
                            I32,
                            tempDoublePtr,
                            MutableGlobals::address + offsetof(MutableGlobals, tempDoublePtr));
    DEFINE_INTRINSIC_GLOBAL(emEnv, "ABORT", I32, ABORT, 0);
    DEFINE_INTRINSIC_GLOBAL(emEnv, "cttz_i8", I32, cttz_i8, 0);
    DEFINE_INTRINSIC_GLOBAL(emEnv, "___dso_handle", U32, ___dso_handle, 0);
    DEFINE_INTRINSIC_GLOBAL(emEnv,
                            "_stderr",
                            I32,
                            _stderr,
                            MutableGlobals::address + offsetof(MutableGlobals, _stderr));
    DEFINE_INTRINSIC_GLOBAL(emEnv,
                            "_stdin",
                            I32,
                            _stdin,
                            MutableGlobals::address + offsetof(MutableGlobals, _stdin));
    DEFINE_INTRINSIC_GLOBAL(emEnv,
                            "_stdout",
                            I32,
                            _stdout,
                            MutableGlobals::address + offsetof(MutableGlobals, _stdout));

    DEFINE_INTRINSIC_GLOBAL(emEnv, "__memory_base", U32, memory_base, 1024);
    DEFINE_INTRINSIC_GLOBAL(emEnv, "memoryBase", U32, emscriptenMemoryBase, 1024);

    DEFINE_INTRINSIC_GLOBAL(emEnv, "__table_base", U32, table_base, 0);
    DEFINE_INTRINSIC_GLOBAL(emEnv, "tableBase", U32, emscriptenTableBase, 0);

    DEFINE_INTRINSIC_GLOBAL(emEnv,
                            "DYNAMICTOP_PTR",
                            U32,
                            DYNAMICTOP_PTR,
                            MutableGlobals::address + offsetof(MutableGlobals, DYNAMICTOP_PTR))
    DEFINE_INTRINSIC_GLOBAL(emEnv, "_environ", U32, em_environ, 0)
    DEFINE_INTRINSIC_GLOBAL(emEnv, "EMTSTACKTOP", U32, EMTSTACKTOP, 0)
    DEFINE_INTRINSIC_GLOBAL(emEnv, "EMT_STACK_MAX", U32, EMT_STACK_MAX, 0)
    DEFINE_INTRINSIC_GLOBAL(emEnv, "eb", I32, eb, 0)
    DEFINE_INTRINSIC_GLOBAL(emEnv, "gb", I32, gb, 0)
    DEFINE_INTRINSIC_GLOBAL(emEnv, "fb", I32, fb, 0)

    DEFINE_INTRINSIC_FUNCTION(emEnv, "getTotalMemory", U32, getTotalMemory) {
        util::getLogger()->debug("S - getTotalMemory");

        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        Uptr memSize = Runtime::getMemoryNumPages(memoryPtr) * IR::numBytesPerPage;
        U32 res = coerce32bitAddress(memoryPtr, memSize);

        util::getLogger()->debug("S - getTotalMemory - {} (dynamic top - {})", res,
                                 getEmscriptenDynamicTop());

        return res;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_emscripten_get_heap_size", U32, _emscripten_get_heap_size) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;

        Uptr memSize = Runtime::getMemoryNumPages(memoryPtr) * IR::numBytesPerPage;
        U32 res = coerce32bitAddress(memoryPtr, memSize);

        util::getLogger()->debug("S - _emscripten_get_heap_size - {} (dynamic top - {})", res,
                                 getEmscriptenDynamicTop());

        return res;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "abortStackOverflow", void, abortStackOverflow, I32 size) {
        util::getLogger()->debug("S - abortStackOverflow - {}", size);

        throwException(Runtime::ExceptionTypes::calledAbort);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "abortOnCannotGrowMemory", U32, abortOnCannotGrowMemory, I32 a) {
        util::getLogger()->debug("S - abortOnCannotGrowMemory - {}", a);

        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "enlargeMemory", I32, enlargeMemory) {
        util::getLogger()->debug("S - enlargeMemory");

        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    /**
     * This function is from the Emscripten asm.js toolchain. It returns true/ false depending
     * on whether it was successful
     */
    DEFINE_INTRINSIC_FUNCTION(emEnv, "_emscripten_resize_heap", I32, _emscripten_resize_heap, U32 size) {
        util::getLogger()->debug("S - _emscripten_resize_heap - {}", size);

        WasmModule *module = getExecutingModule();
        Runtime::Memory *memory = module->defaultMemory;
        Uptr initialPages = getMemoryNumPages(memory);
        Uptr initialSize = initialPages * IR::numBytesPerPage;

        // Grow the memory accordingly
        int expansionRes = expandMemory(size);

        if (expansionRes == EXPAND_TOO_BIG) {
            Uptr maxPages = getMemoryMaxPages(memory);
            Uptr maxBytes = maxPages * IR::numBytesPerPage;

            util::getLogger()->error("Emscripten resize heap failed (max bytes {}, requested {})", maxBytes, size);
            throw std::runtime_error("Emscripten expanding beyond max heap size");

        } else if (expansionRes == EXPAND_NO_ACTION) {
            util::getLogger()->debug("Emscripten requested pointless resize heap (requested {}, current {})", size,
                                     initialSize);
            return 0;
        }

        // In emscripten, must now set the dynamic top ptr to the requested resize point, NOT the top of the memory
        setEmscriptenDynamicTop(size);

        Uptr newPages = getMemoryNumPages(memory);
        util::getLogger()->debug("Emscripten heap {} -> {} (dynamic top {})", initialPages, newPages,
                                 getEmscriptenDynamicTop());

        return 1;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_time", I32, _time, U32 address) {
        util::getLogger()->debug("S - _time - {}", address);

        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        time_t t = time(nullptr);
        if (address) { Runtime::memoryRef<I32>(memoryPtr, address) = (I32) t; }
        return (I32) t;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___errno_location", I32, ___errno_location) {
        util::getLogger()->debug("S - ___errno_location");
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___setErrNo", void, ___seterrno, I32 value) {
        util::getLogger()->debug("S - ___setErrNo = {}", value);
        if (EMSCRIPTEN_ERRNO_LOCATION) {
            Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
            Runtime::memoryRef<I32>(memoryPtr, EMSCRIPTEN_ERRNO_LOCATION) = (I32) value;
        }
    }

    /**
     * Allowing straight-through access to sysconf my not be wise. Should revisit this.
     */
    DEFINE_INTRINSIC_FUNCTION(emEnv, "_sysconf", I32, _sysconf, I32 a) {
        util::getLogger()->debug("S - _sysconf - {}", a);

        return sysconf(a);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___ctype_b_loc", U32, ___ctype_b_loc) {
        unsigned short data[384] = {
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2,
                2, 2, 2, 2, 2, 2, 2, 8195, 8194, 8194, 8194, 8194, 2,
                2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                2, 2, 2, 2, 24577, 49156, 49156, 49156, 49156, 49156, 49156, 49156, 49156,
                49156, 49156, 49156, 49156, 49156, 49156, 49156, 55304, 55304, 55304, 55304, 55304, 55304,
                55304, 55304, 55304, 55304, 49156, 49156, 49156, 49156, 49156, 49156, 49156, 54536, 54536,
                54536, 54536, 54536, 54536, 50440, 50440, 50440, 50440, 50440, 50440, 50440, 50440, 50440,
                50440, 50440, 50440, 50440, 50440, 50440, 50440, 50440, 50440, 50440, 50440, 49156, 49156,
                49156, 49156, 49156, 49156, 54792, 54792, 54792, 54792, 54792, 54792, 50696, 50696, 50696,
                50696, 50696, 50696, 50696, 50696, 50696, 50696, 50696, 50696, 50696, 50696, 50696, 50696,
                50696, 50696, 50696, 50696, 49156, 49156, 49156, 49156, 2, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0};
        static U32 vmAddress = 0;

        if (vmAddress == 0) {
            Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;

            vmAddress = coerce32bitAddress(memoryPtr, dynamicAlloc(memoryPtr, sizeof(data)));
            memcpy(Runtime::memoryArrayPtr<U8>(memoryPtr, vmAddress, sizeof(data)), data, sizeof(data));
        }

        return vmAddress + sizeof(short) * 128;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___ctype_toupper_loc", U32, ___ctype_toupper_loc) {
        I32 data[384]
                = {128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145,
                   146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163,
                   164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181,
                   182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199,
                   200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217,
                   218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235,
                   236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253,
                   254, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                   16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
                   34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
                   52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
                   70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87,
                   88, 89, 90, 91, 92, 93, 94, 95, 96, 65, 66, 67, 68, 69, 70, 71, 72, 73,
                   74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 123,
                   124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141,
                   142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
                   160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177,
                   178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195,
                   196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213,
                   214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231,
                   232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249,
                   250, 251, 252, 253, 254, 255};
        static U32 vmAddress = 0;

        if (vmAddress == 0) {
            Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
            vmAddress = coerce32bitAddress(memoryPtr, dynamicAlloc(memoryPtr, sizeof(data)));
            memcpy(Runtime::memoryArrayPtr<U8>(memoryPtr, vmAddress, sizeof(data)), data, sizeof(data));
        }

        return vmAddress + sizeof(I32) * 128;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___ctype_tolower_loc", U32, ___ctype_tolower_loc) {
        I32 data[384]
                = {128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145,
                   146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163,
                   164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181,
                   182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199,
                   200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217,
                   218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235,
                   236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253,
                   254, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                   16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
                   34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
                   52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 97, 98, 99, 100, 101,
                   102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
                   120, 121, 122, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105,
                   106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123,
                   124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141,
                   142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
                   160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177,
                   178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195,
                   196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213,
                   214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231,
                   232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249,
                   250, 251, 252, 253, 254, 255};
        static U32 vmAddress = 0;

        if (vmAddress == 0) {
            Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
            vmAddress = coerce32bitAddress(memoryPtr, dynamicAlloc(memoryPtr, sizeof(data)));
            memcpy(Runtime::memoryArrayPtr<U8>(memoryPtr, vmAddress, sizeof(data)), data, sizeof(data));
        }

        return vmAddress + sizeof(I32) * 128;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv,
                              "___assert_fail",
                              void,
                              ___assert_fail,
                              I32 condition,
                              I32 filename,
                              I32 line,
                              I32 function) {
        util::getLogger()->debug("S - ___assert_fail - {} {} {} {}", condition, filename, line, function);

        throwException(Runtime::ExceptionTypes::calledAbort);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_abort", void, emscripten__abort) {
        util::getLogger()->debug("S - _abort");

        throwException(Runtime::ExceptionTypes::calledAbort);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_exit", void, emscripten__exit, I32 code) {
        util::getLogger()->debug("S - _exit - {}", code);
        throwException(Runtime::ExceptionTypes::calledAbort);
    }

//    DEFINE_INTRINSIC_FUNCTION(emEnv, "abort", void, emscripten_abort, I32 a) {
//        util::getLogger()->debug("S - abort {}", a);
//
//        throwException(Runtime::ExceptionTypes::calledAbort);
//    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "abort", void, emscripten_abort) {
        util::getLogger()->debug("S - abort");

        throwException(Runtime::ExceptionTypes::calledAbort);
    }

    /**
     * All the null funcs are catastrophic and related to issues with function pointers
     * https://emscripten.org/docs/porting/Debugging.html#function-pointer-issues
     *
     * You need to make sure you link your code with -s EMULATE_FUNCTION_POINTER_CASTS=1
     */
    DEFINE_INTRINSIC_FUNCTION(emEnv, "nullFunc_X", void, emscripten_nullFunc_X, I32 code) {
        util::getLogger()->debug("S - nullFunc_X - {}", code);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "nullFunc_i", void, emscripten_nullFunc_i, I32 code) {
        util::getLogger()->debug("S - nullFunc_i - {}", code);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "nullFunc_ii", void, emscripten_nullFunc_ii, I32 code) {
        util::getLogger()->debug("S - nullFunc_ii - {}", code);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "nullFunc_iii", void, emscripten_nullFunc_iii, I32 code) {
        util::getLogger()->debug("S - nullFunc_iii - {}", code);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "nullFunc_iiii", void, emscripten_nullFunc_iiii, I32 code) {
        util::getLogger()->debug("S - nullFunc_iiii - {}", code);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "nullFunc_iiiii", void, emscripten_nullFunc_iiiii, I32 code) {
        util::getLogger()->debug("S - nullFunc_iiiii - {}", code);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "nullFunc_iiiiii", void, emscripten_nullFunc_iiiiii, I32 code) {
        util::getLogger()->debug("S - nullFunc_iiiiii - {}", code);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "nullFunc_iiiiiii", void, emscripten_nullFunc_iiiiiii, I32 code) {
        util::getLogger()->debug("S - nullFunc_iiiiiii - {}", code);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "nullFunc_v", void, emscripten_nullFunc_v, I32 code) {
        util::getLogger()->debug("S - nullFunc_v - {}", code);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "nullFunc_vi", void, emscripten_nullFunc_vi, I32 code) {
        util::getLogger()->debug("S - nullFunc_vi - {}", code);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "nullFunc_vii", void, emscripten_nullFunc_vii, I32 code) {
        util::getLogger()->debug("S - nullFunc_vii - {}", code);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "nullFunc_viii", void, emscripten_nullFunc_viii, I32 code) {
        util::getLogger()->debug("S - nullFunc_viii - {}", code);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "nullFunc_viiii", void, emscripten_nullFunc_viiii, I32 code) {
        util::getLogger()->debug("S - nullFunc_viiii - {}", code);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "nullFunc_viiiii", void, emscripten_nullFunc_viiiii, I32 code) {
        util::getLogger()->debug("S - nullFunc_viiiii - {}", code);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "nullFunc_viiiiii", void, emscripten_nullFunc_viiiiii, I32 code) {
        util::getLogger()->debug("S - nullFunc_viiiiii - {}", code);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "nullFunc_dd", void, emscripten_nullFunc_dd, I32 code) {
        util::getLogger()->debug("S - nullFunc_dd - {}", code);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "nullFunc_ddd", void, emscripten_nullFunc_ddd, I32 code) {
        util::getLogger()->debug("S - nullFunc_ddd - {}", code);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    static U32 currentLocale = 0;

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_uselocale", I32, _uselocale, I32 locale) {
        util::getLogger()->debug("S - _uselocale - {}", locale);

        auto oldLocale = currentLocale;
        currentLocale = locale;
        return oldLocale;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_newlocale", U32, _newlocale, I32 mask, I32 locale, I32 base) {
        util::getLogger()->debug("S - _newlocale - {} {} {}", mask, locale, base);

        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        if (!base) { base = coerce32bitAddress(memoryPtr, dynamicAlloc(memoryPtr, 4)); }
        return base;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_freelocale", void, emscripten__freelocale, I32 a) {
        util::getLogger()->debug("S - _freelocale");
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_strftime_l", I32, emscripten__strftime_l, I32 a, I32 b, I32 c, I32 d, I32 e) {
        util::getLogger()->debug("S - _strftime_l - {} {} {} {} {}", a, b, c, d, e);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_strerror", I32, emscripten__strerror, I32 a) {
        util::getLogger()->debug("S - _strerror - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_catopen", I32, emscripten__catopen, I32 a, I32 b) {
        util::getLogger()->debug("S - _catopen - {} {}", a, b);
        return -1;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_catgets", I32, emscripten__catgets, I32 catd, I32 set_id, I32 msg_id, I32 s) {
        util::getLogger()->debug("S - _catgets - {} {} {} {}", catd, set_id, msg_id, s);

        return s;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_catclose", I32, emscripten__catclose, I32 a) {
        util::getLogger()->debug("S - _catclose - {}", a);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_emscripten_memcpy_big", U32, _emscripten_memcpy_big, U32 destPtr, U32 srcPtr,
                              U32 numBytes) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;

        util::getLogger()->debug("S - _emscripten_memcpy_big - {} {} {} (dynamic top {})", destPtr, srcPtr, numBytes,
                                 getEmscriptenDynamicTop());

        U8 *dest = Runtime::memoryArrayPtr<U8>(memoryPtr, destPtr, numBytes);
        U8 *src = Runtime::memoryArrayPtr<U8>(memoryPtr, srcPtr, numBytes);

        std::copy(src, src + numBytes, dest);

        return destPtr;
    }

    FILE *vmFile(U32 vmHandle) {
        switch ((ioStreamVMHandle) vmHandle) {
            case ioStreamVMHandle::StdErr:
                return stderr;
            case ioStreamVMHandle::StdIn:
                return stdin;
            case ioStreamVMHandle::StdOut:
                return stdout;
            default:
                return stdout; // std::cerr << "invalid file handle " << vmHandle << std::endl; throw;
        }
    }

    /**
     * fprintf can provide some useful debugging info so we can just spit it to stdout
     */
    DEFINE_INTRINSIC_FUNCTION(emEnv,
                              "_vfprintf",
                              I32,
                              _vfprintf,
                              I32 fd,
                              U32 formatPtr,
                              I32 argList) {

        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        const char *format = &Runtime::memoryRef<char>(memoryPtr, (Uptr) formatPtr);
        std::cout << "S - vfprintf - " << format << std::endl;
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_getc", I32, _getc, I32 file) {
        util::getLogger()->debug("S - _getc - {}", file);

        return getc(vmFile(file));
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_ungetc", I32, _ungetc, I32 character, I32 file) {
        util::getLogger()->debug("S - _ungetc - {}", file);

        return ungetc(character, vmFile(file));
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_fread", U32, _fread, U32 destAddress, U32 size, U32 count, I32 file) {
        util::getLogger()->debug("S - _fread - {} {} {} {}", destAddress, size, count, file);

        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;

        U8 *destPtr = Runtime::memoryArrayPtr<U8>(memoryPtr, destAddress, U64(size) * U64(count));
        size_t addr = fread(destPtr, U64(size), U64(count), vmFile(file));

        return coerce32bitAddress(memoryPtr, addr);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_fwrite", U32, _fwrite, U32 sourceAddress, U32 size, U32 count, I32 file) {
        util::getLogger()->debug("S - _fwrite - {} {} {} {}", sourceAddress, size, count, file);

        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        U8 *sourcePtr = Runtime::memoryArrayPtr<U8>(memoryPtr, sourceAddress, U64(size) * U64(count));

        return coerce32bitAddress(memoryPtr, fwrite(sourcePtr, U64(size), U64(count), vmFile(file)));
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_fputc", I32, _fputc, I32 character, I32 file) {
        util::getLogger()->debug("S - _fputc - {} {}", character, file);

        return fputc(character, vmFile(file));
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_fflush", I32, _fflush, I32 file) {
        util::getLogger()->debug("S - _fflush - {}", file);
        return fflush(vmFile(file));
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___lock", void, ___lock, I32 a) {
        util::getLogger()->debug("S - ___lock - {}", a);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___unlock", void, ___unlock, I32 a) {
        util::getLogger()->debug("S - ___unlock - {}", a);

    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___lockfile", I32, ___lockfile, I32 a) {
        util::getLogger()->debug("S - ___lockfile - {}", a);

        return 1;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___unlockfile", void, ___unlockfile, I32 a) {
        util::getLogger()->debug("S - ___unlockfile - {}", a);

    }

    static F64 makeNaN() {
        FloatComponents<F64> floatBits;
        floatBits.bits.sign = 0;
        floatBits.bits.exponent = FloatComponents<F64>::maxExponentBits;
        floatBits.bits.significand = FloatComponents<F64>::canonicalSignificand;
        return floatBits.value;
    }

    static F64 makeInf() {
        FloatComponents<F64> floatBits;
        floatBits.bits.sign = 0;
        floatBits.bits.exponent = FloatComponents<F64>::maxExponentBits;
        floatBits.bits.significand = FloatComponents<F64>::maxSignificand;
        return floatBits.value;
    }

    DEFINE_INTRINSIC_GLOBAL(emGlobal, "NaN", F64, NaN, makeNaN())
    DEFINE_INTRINSIC_GLOBAL(emGlobal, "Infinity", F64, Infinity, makeInf())

    DEFINE_INTRINSIC_FUNCTION(emGlobal, "pow", F64, mathPow, F64 a, F64 b) {
        util::getLogger()->debug("S - pow - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    // ---------------------------------------
    // pthreads
    // ---------------------------------------

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_pthread_attr_destroy", I32, _pthread_attr_destroy, I32 a) {
        util::getLogger()->debug("S - _pthread_attr_destroy - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_pthread_attr_init", I32, _pthread_attr_init, I32 a) {
        util::getLogger()->debug("S - _pthread_attr_init - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_pthread_attr_setstacksize", I32, _pthread_attr_setstacksize, I32 a, I32 b) {
        util::getLogger()->debug("S - _pthread_attr_setstacksize - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_pthread_cleanup_pop", void, _pthread_cleanup_pop, I32 a) {
        util::getLogger()->debug("S - _pthread_cleanup_pop - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_pthread_cleanup_push", void, _pthread_cleanup_push, I32 a, I32 b) {
        util::getLogger()->debug("S - _pthread_cleanup_push - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_pthread_cond_destroy", I32, _pthread_cond_destroy, I32 a) {
        util::getLogger()->debug("S - _pthread_cond_destroy - {}", a);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_pthread_cond_init", I32, _pthread_cond_init, I32 a, I32 b) {
        util::getLogger()->debug("S - _pthread_cond_init - {} {}", a, b);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_pthread_cond_signal", I32, _pthread_cond_signal, I32 a) {
        util::getLogger()->debug("S - _pthread_cond_signal - {}", a);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_pthread_cond_timedwait", I32, _pthread_cond_timedwait, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - _pthread_cond_timedwait - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_pthread_cond_wait", I32, _pthread_cond_wait, I32 a, I32 b) {
        util::getLogger()->debug("S - _pthread_cond_wait - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_pthread_create", I32, _pthread_create, I32 a, I32 b, I32 c, I32 d) {
        util::getLogger()->debug("S - _pthread_create - {} {} {} {}", a, b, c, d);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_pthread_detach", I32, _pthread_detach, I32 a) {
        util::getLogger()->debug("S - _pthread_detach - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_pthread_equal", I32, _pthread_equal, I32 a, I32 b) {
        util::getLogger()->debug("S - _pthread_equal - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_pthread_exit", void, _pthread_exit, I32 a) {
        util::getLogger()->debug("S - _pthread_exit - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_pthread_getspecific", I32, _pthread_getspecific, I32 a) {
        util::getLogger()->debug("S - _pthread_getspecific - {}", a);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_pthread_join", I32, _pthread_join, I32 a, I32 b) {
        util::getLogger()->debug("S - _pthread_join - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_pthread_key_create", I32, _pthread_key_create, I32 keyPtr, I32 destructor) {
        util::getLogger()->debug("S - _pthread_key_create - {} {}", keyPtr, destructor);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_pthread_key_delete", I32, _pthread_key_delete, I32 a) {
        util::getLogger()->debug("S - _pthread_key_delete - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_pthread_mutex_destroy", I32, _pthread_mutex_destroy, I32 a) {
        util::getLogger()->debug("S - _pthread_mutex_destroy - {}", a);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_pthread_mutex_init", I32, _pthread_mutex_init, I32 a, I32 b) {
        util::getLogger()->debug("S - _pthread_mutex_init - {} {}", a, b);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_pthread_mutexattr_destroy", I32, _pthread_mutexattr_destroy, I32 a) {
        util::getLogger()->debug("S - _pthread_mutexattr_destroy - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_pthread_mutexattr_init", I32, _pthread_mutexattr_init, I32 a) {
        util::getLogger()->debug("S - _pthread_mutexattr_init - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_pthread_mutexattr_settype", I32, _pthread_mutexattr_settype, I32 a, I32 b) {
        util::getLogger()->debug("S - _pthread_mutexattr_settype - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_pthread_once", I32, _pthread_once, I32 a, I32 b) {
        util::getLogger()->debug("S - _pthread_once - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_pthread_setspecific", I32, _pthread_setspecific, I32 a, I32 b) {
        util::getLogger()->debug("S - _pthread_setspecific - {} {}", a, b);

        return 0;
    }

    // ---------------------------------
    // Uncategorised (mainly dumped from Python trace)
    // ---------------------------------

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall300", I32, ___syscall300, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall300 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall12", I32, ___syscall12, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall12 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall63", I32, ___syscall63, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall63 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall340", I32, ___syscall340, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall340 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall306", I32, ___syscall306, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall306 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall295", I32, ___syscall295, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall295 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall212", I32, ___syscall212, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall212 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall15", I32, ___syscall15, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall15 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall132", I32, ___syscall132, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall132 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_dlsym", I32, _dlsym, I32 a, I32 b) {
        util::getLogger()->debug("S - _dlsym - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_confstr", I32, _confstr, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - _confstr - {} {} {}", a, b, c);

        // Return zero as if no confstr variables have a value set
        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_clock_settime", I32, _clock_settime, I32 a, I32 b) {
        util::getLogger()->debug("S - _clock_settime - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_chroot", I32, _chroot, I32 a) {
        util::getLogger()->debug("S - _chroot - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_alarm", I32, _alarm, I32 a) {
        util::getLogger()->debug("S - _alarm - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___wait", void, ___wait, I32 a, I32 b, I32 c, I32 d) {
        util::getLogger()->debug("S - ___wait - {} {} {} {}", a, b, c, d);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall97", I32, ___syscall97, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall97 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall96", I32, ___syscall96, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall96 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall94", I32, ___syscall94, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall94 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall9", I32, ___syscall9, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall9 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall83", I32, ___syscall83, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall83 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall77", I32, ___syscall77, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall77 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall75", I32, ___syscall75, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall75 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall66", I32, ___syscall66, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall66 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall118", I32, ___syscall118, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall118 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall64", I32, ___syscall64, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall64 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall60", I32, ___syscall60, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall60 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall57", I32, ___syscall57, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall57 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall42", I32, ___syscall42, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall42 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall40", I32, ___syscall40, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall40 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall36", I32, ___syscall36, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall36 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall34", I32, ___syscall34, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall34 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall331", I32, ___syscall331, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall331 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall330", I32, ___syscall330, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall330 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall33", I32, ___syscall33, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall33 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall324", I32, ___syscall324, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall324 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall320", I32, ___syscall320, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall320 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall305", I32, ___syscall305, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall305 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall304", I32, ___syscall304, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall304 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall303", I32, ___syscall303, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall303 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall302", I32, ___syscall302, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall302 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall301", I32, ___syscall301, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall301 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall298", I32, ___syscall298, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall298 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall297", I32, ___syscall297, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall297 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall296", I32, ___syscall296, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall296 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall29", I32, ___syscall29, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall29 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall272", I32, ___syscall272, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall272 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall269", I32, ___syscall269, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall269 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall268", I32, ___syscall268, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall268 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall211", I32, ___syscall211, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall211 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall209", I32, ___syscall209, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall209 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall207", I32, ___syscall207, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall207 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall205", I32, ___syscall205, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall205 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall20", I32, ___syscall20, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall20 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall198", I32, ___syscall198, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall198 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___libc_current_sigrtmin", I32, ___libc_current_sigrtmin) {
        util::getLogger()->debug("S - ___libc_current_sigrtmin");
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall194", I32, ___syscall194, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall194 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall193", I32, ___syscall193, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall193 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall191", I32, ___syscall191, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall191 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall181", I32, ___syscall181, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall181 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall180", I32, ___syscall180, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall180 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___libc_current_sigrtmax", I32, ___libc_current_sigrtmax) {
        util::getLogger()->debug("S - ___libc_current_sigrtmax");
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall148", I32, ___syscall148, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall148 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall147", I32, ___syscall147, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall147 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_waitid", I32, _waitid, I32 a, I32 b, I32 c, I32 d) {
        util::getLogger()->debug("S - _waitid - {} {} {} {}", a, b, c, d);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_wait4", I32, _wait4, I32 a, I32 b, I32 c, I32 d) {
        util::getLogger()->debug("S - _wait4 - {} {} {} {}", a, b, c, d);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_wait3", I32, _wait3, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - _wait3 - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_wait", I32, _wait, I32 a) {
        util::getLogger()->debug("S - _wait - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall142", I32, ___syscall142, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall142 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_system", I32, _system, I32 a) {
        util::getLogger()->debug("S - _system - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_strftime", I32, _strftime, I32 a, I32 b, I32 c, I32 d) {
        util::getLogger()->debug("S - _strftime - {} {} {} {}", a, b, c, d);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_sigpending", I32, _sigpending, I32 a) {
        util::getLogger()->debug("S - _sigpending - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_sigismember", I32, _sigismember, I32 a, I32 b) {
        util::getLogger()->debug("S - _sigismember - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_siginterrupt", I32, _siginterrupt, I32 a, I32 b) {
        util::getLogger()->debug("S - _siginterrupt - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_sigfillset", I32, _sigfillset, I32 a) {
        util::getLogger()->debug("S - _sigfillset - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_setpwent", void, _setpwent) {
        util::getLogger()->debug("S - _setpwent");
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_setitimer", I32, _setitimer, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - _setitimer - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_setgroups", I32, _setgroups, I32 a, I32 b) {
        util::getLogger()->debug("S - _setgroups - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_sched_yield", I32, _sched_yield) {
        util::getLogger()->debug("S - _sched_yield");
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_putenv", I32, _putenv, I32 a) {
        util::getLogger()->debug("S - _putenv - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall14", I32, ___syscall14, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall14 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_pathconf", I32, _pathconf, I32 a, I32 b) {
        util::getLogger()->debug("S - _pathconf - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_killpg", I32, _killpg, I32 a, I32 b) {
        util::getLogger()->debug("S - _killpg - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_kill", I32, _kill, I32 a, I32 b) {
        util::getLogger()->debug("S - _kill - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_gmtime", I32, _gmtime, I32 a) {
        util::getLogger()->debug("S - _gmtime - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall133", I32, ___syscall133, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall133 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_getpwnam", I32, _getpwnam, I32 a) {
        util::getLogger()->debug("S - _getpwnam - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_getloadavg", I32, _getloadavg, I32 a, I32 b) {
        util::getLogger()->debug("S - _getloadavg - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_getitimer", I32, _getitimer, I32 a, I32 b) {
        util::getLogger()->debug("S - _getitimer - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_fexecve", I32, _fexecve, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - _fexecve - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_execve", I32, _execve, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - _execve - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_execv", I32, _execv, I32 a, I32 b) {
        util::getLogger()->debug("S - _execv - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_dlopen", I32, _dlopen, I32 a, I32 b) {
        util::getLogger()->debug("S - _dlopen - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_dlerror", I32, _dlerror) {
        util::getLogger()->debug("S - _dlerror");
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall122", I32, ___syscall122, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall122 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___buildEnvironment", void, ___buildEnvironment, I32 a) {
        util::getLogger()->debug("S - ___buildEnvironment - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_emscripten_async_wget", void, _emscripten_async_wget, I32 a, I32 b, I32 c,
                              I32 d) {
        util::getLogger()->debug("S - _emscripten_async_wget - {} {} {} {}", a, b, c, d);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_emscripten_exit_with_live_runtime", void, _emscripten_exit_with_live_runtime) {
        util::getLogger()->debug("S - _emscripten_exit_with_live_runtime");
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "setTempRet0", void, setTempRet0, I32 a) {
        util::getLogger()->debug("S - setTempRet0 - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "getTempRet0", I32, getTempRet0) {
        util::getLogger()->debug("S - getTempRet0");
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "invoke_ii", I32, invoke_ii, I32 a, I32 b) {
        util::getLogger()->debug("S - invoke_ii - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "invoke_iii", I32, invoke_iii, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - invoke_iii - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "invoke_iiii", I32, invoke_iiii, I32 a, I32 b, I32 c, I32 d) {
        util::getLogger()->debug("S - invoke_iiii - {} {} {} {}", a, b, c, d);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "invoke_iiiii", I32, invoke_iiiii, I32 a, I32 b, I32 c, I32 d, I32 e) {
        util::getLogger()->debug("S - invoke_iiiii - {} {} {} {} {}", a, b, c, d, e);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "invoke_vi", void, invoke_vi, I32 a, I32 b) {
        util::getLogger()->debug("S - invoke_vi - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "invoke_vii", void, invoke_vii, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - invoke_vii - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "invoke_viiii", void, invoke_viiii, I32 a, I32 b, I32 c, I32 d, I32 e) {
        util::getLogger()->debug("S - invoke_viiii - {} {} {} {} {}", a, b, c, d, e);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___clock_gettime", I32, ___clock_gettime, I32 a, I32 b) {
        util::getLogger()->debug("S - ___clock_gettime - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___cxa_allocate_exception", I32, ___cxa_allocate_exception, I32 a) {
        util::getLogger()->debug("S - ___cxa_allocate_exception - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___cxa_begin_catch", I32, ___cxa_begin_catch, I32 a) {
        util::getLogger()->debug("S - ___cxa_begin_catch - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___cxa_current_primary_exception", I32, ___cxa_current_primary_exception) {
        util::getLogger()->debug("S - ___cxa_current_primary_exception");
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___cxa_decrement_exception_refcount", void, ___cxa_decrement_exception_refcount,
                              I32 a) {
        util::getLogger()->debug("S - ___cxa_decrement_exception_refcount - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___cxa_increment_exception_refcount", void, ___cxa_increment_exception_refcount,
                              I32 a) {
        util::getLogger()->debug("S - ___cxa_increment_exception_refcount - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___cxa_pure_virtual", void, ___cxa_pure_virtual) {
        util::getLogger()->debug("S - ___cxa_pure_virtual");
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___cxa_rethrow_primary_exception", void, ___cxa_rethrow_primary_exception,
                              I32 a) {
        util::getLogger()->debug("S - ___cxa_rethrow_primary_exception - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___cxa_throw", void, ___cxa_throw, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - ___cxa_throw - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___cxa_uncaught_exception", I32, ___cxa_uncaught_exception) {
        util::getLogger()->debug("S - ___cxa_uncaught_exception");
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall114", I32, ___syscall114, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall114 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall121", I32, ___syscall121, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall121 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall125", I32, ___syscall125, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall125 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall144", I32, ___syscall144, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall144 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall150", I32, ___syscall150, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall150 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall151", I32, ___syscall151, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall151 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall152", I32, ___syscall152, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall152 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall153", I32, ___syscall153, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall153 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall163", I32, ___syscall163, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall163 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall218", I32, ___syscall218, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall218 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall219", I32, ___syscall219, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall219 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall308", I32, ___syscall308, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall308 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall333", I32, ___syscall333, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall333 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall334", I32, ___syscall334, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall334 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall337", I32, ___syscall337, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall337 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall345", I32, ___syscall345, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall345 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "___syscall51", I32, ___syscall51, I32 a, I32 b) {
        util::getLogger()->debug("S - ___syscall51 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_emscripten_longjmp", void, _emscripten_longjmp, I32 a, I32 b) {
        util::getLogger()->debug("S - _emscripten_longjmp - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_gai_strerror", I32, _gai_strerror, I32 a) {
        util::getLogger()->debug("S - _gai_strerror - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_getaddrinfo", I32, _getaddrinfo, I32 a, I32 b, I32 c, I32 d) {
        util::getLogger()->debug("S - _getaddrinfo - {} {} {} {}", a, b, c, d);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_gethostbyaddr", I32, _gethostbyaddr, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - _gethostbyaddr - {} {} {}", a, b, c);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_getnameinfo", I32, _getnameinfo, I32 a, I32 b, I32 c, I32 d, I32 e, I32 f,
                              I32 g) {
        util::getLogger()->debug("S - _getnameinfo - {} {} {} {} {} {} {}", a, b, c, d, e, f, g);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_getprotobyname", I32, _getprotobyname, I32 a) {
        util::getLogger()->debug("S - _getprotobyname - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_gmtime_r", I32, _gmtime_r, I32 a, I32 b) {
        util::getLogger()->debug("S - _gmtime_r - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_inet_addr", I32, _inet_addr, I32 a) {
        util::getLogger()->debug("S - _inet_addr - {}", a);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_llvm_copysign_f32", F64, _llvm_copysign_f32, F64 a, F64 b) {
        util::getLogger()->debug("S - _llvm_copysign_f32 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_llvm_copysign_f64", F64, _llvm_copysign_f64, F64 a, F64 b) {
        util::getLogger()->debug("S - _llvm_copysign_f64 - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_llvm_trap", void, _llvm_trap) {
        util::getLogger()->debug("S - _llvm_trap");
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_localtime_r", I32, _localtime_r, I32 a, I32 b) {
        util::getLogger()->debug("S - _localtime_r - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_longjmp", void, _longjmp, I32 a, I32 b) {
        util::getLogger()->debug("S - _longjmp - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_nanosleep", I32, _nanosleep, I32 a, I32 b) {
        util::getLogger()->debug("S - _nanosleep - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_sigaddset", I32, _sigaddset, I32 a, I32 b) {
        util::getLogger()->debug("S - _sigaddset - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_signal", I32, _signal, I32 a, I32 b) {
        util::getLogger()->debug("S - _signal - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    DEFINE_INTRINSIC_FUNCTION(emEnv, "_utime", I32, _utime, I32 a, I32 b) {
        util::getLogger()->debug("S - _utime - {} {}", a, b);
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }
}