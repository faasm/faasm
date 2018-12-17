#include "wasm.h"

#include <fcntl.h>
#include <math.h>
#include <netdb.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/uio.h>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/RuntimeData.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <stdarg.h>
#include <prof/prof.h>

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
    static Intrinsics::Module envModule;

    Intrinsics::Module &getIntrinsicModule_env() {
        return envModule;
    }

    static const char *HOSTS_FILE = "/usr/local/faasm/net/hosts";
    static const char *RESOLV_FILE = "/usr/local/faasm/net/resolv.conf";

    // Thread-local variables to isolate bits of environment
    static thread_local std::set<int> openFds;

    std::vector<uint8_t> getBytesFromWasm(I32 dataPtr, I32 dataLen) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        U8 *data = Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr) dataPtr, (Uptr) dataLen);

        std::vector<uint8_t> bytes(data, data + dataLen);

        return bytes;
    }

    std::string getStringFromWasm(I32 strPtr) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        char *key = &Runtime::memoryRef<char>(memoryPtr, (Uptr) strPtr);
        std::string str(key);

        return str;
    }

    std::string getKeyFromWasm(I32 keyPtr) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        char *key = &Runtime::memoryRef<char>(memoryPtr, (Uptr) keyPtr);

        const message::Message *call = getExecutingCall();
        std::string prefixedKey = call->user() + "_" + key;

        return prefixedKey;
    }

    int copyToWasmBuffer(const std::vector<uint8_t> &dataIn, I32 bufferPtr, I32 bufferLen) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        U8 *buffer = Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr) bufferPtr, (Uptr) bufferLen);

        int dataSize = util::safeCopyToBuffer(dataIn, buffer, bufferLen);

        return dataSize;
    }

    // ------------------------
    // FAASM-specific
    // ------------------------
    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_write_state", void, __faasm_write_state,
                              I32 keyPtr, I32 dataPtr, I32 dataLen, I32 async) {
        util::getLogger()->debug("S - write_state - {} {} {}", keyPtr, dataPtr, dataLen);

        // Set the whole state
        std::string key = getKeyFromWasm(keyPtr);
        infra::State &s = infra::getGlobalState();
        infra::StateKeyValue *kv = s.getKV(key);

        // Read the data and set
        const std::vector<uint8_t> newState = getBytesFromWasm(dataPtr, dataLen);
        kv->set(newState);

        // Push if synchronous
        if (async == 0) {
            kv->pushFull();
        }
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_write_state_offset", void, __faasm_write_state_offset,
                              I32 keyPtr, I32 offset, I32 dataPtr, I32 dataLen, I32 async) {
        util::getLogger()->debug("S - write_state_offset - {} {} {} {}", keyPtr, offset, dataPtr, dataLen);

        std::string key = getKeyFromWasm(keyPtr);

        // Set the state at the given offset
        infra::State &s = infra::getGlobalState();
        infra::StateKeyValue *kv = s.getKV(key);

        // Read data and set
        const std::vector<uint8_t> newState = getBytesFromWasm(dataPtr, dataLen);
        kv->setSegment(offset, newState);

        // Push if synchronous
        if (async == 0) {
            kv->pushPartial();
        }
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_read_state", I32, __faasm_read_state,
                              I32 keyPtr, I32 bufferPtr, I32 bufferLen, I32 async) {
        util::getLogger()->debug("S - read_state - {} {} {}", keyPtr, bufferPtr, bufferLen);

        std::string key = getKeyFromWasm(keyPtr);

        // Read the state in
        infra::State &s = infra::getGlobalState();
        infra::StateKeyValue *kv = s.getKV(key);

        // Pull from remote if synchronous
        if (async == 0) {
            kv->pull();
        }

        std::vector<uint8_t> value = kv->get();

        int stateSize = copyToWasmBuffer(value, bufferPtr, bufferLen);

        // Return the total number of bytes in the whole state
        return stateSize;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_read_state_offset", void, __faasm_read_state_offset,
                              I32 keyPtr, I32 offset, I32 bufferPtr, I32 bufferLen, I32 async) {
        util::getLogger()->debug("S - read_state_offset - {} {} {} {}", keyPtr, offset, bufferPtr, bufferLen);

        std::string key = getKeyFromWasm(keyPtr);

        // Read the state in
        infra::State &s = infra::getGlobalState();
        infra::StateKeyValue *kv = s.getKV(key);

        // Pull from remote if synchronous
        if (async == 0) {
            kv->pull();
        }

        std::vector<uint8_t> value = kv->getSegment(offset, bufferLen);

        copyToWasmBuffer(value, bufferPtr, bufferLen);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_read_input", I32, __faasm_read_input, I32 bufferPtr, I32 bufferLen) {
        util::getLogger()->debug("S - read_input - {} {}", bufferPtr, bufferLen);

        // Get the input
        message::Message *call = getExecutingCall();
        std::vector<uint8_t> inputBytes = util::stringToBytes(call->inputdata());

        // Write to the wasm buffer
        int inputSize = copyToWasmBuffer(inputBytes, bufferPtr, bufferLen);

        return inputSize;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_write_output", void, __faasm_write_output, I32 outputPtr, I32 outputLen) {
        util::getLogger()->debug("S - write_output - {} {}", outputPtr, outputLen);

        std::vector<uint8_t> outputData = getBytesFromWasm(outputPtr, outputLen);
        message::Message *call = getExecutingCall();
        call->set_outputdata(outputData.data(), outputData.size());
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_chain_function", void, __faasm_chain_function,
                              I32 namePtr, I32 inputDataPtr, I32 inputDataLen) {
        util::getLogger()->debug("S - chain_function - {} {} {}", namePtr, inputDataPtr, inputDataLen);

        message::Message *call = getExecutingCall();
        CallChain *callChain = getExecutingCallChain();
        std::string funcName = getStringFromWasm(namePtr);
        const std::vector<uint8_t> inputData = getBytesFromWasm(inputDataPtr, inputDataLen);

        // Add this to the chain of calls
        callChain->addCall(call->user(), funcName, inputData);
    }

    // ------------------------
    // I/O - supported
    // ------------------------

    void checkThreadOwnsFd(int fd) {
        const std::shared_ptr<spdlog::logger> logger = util::getLogger();
        bool isNotOwned = openFds.find(fd) == openFds.end();

        if (fd == STDIN_FILENO) {
            logger->error("Process interacting with stdin");
            throw std::runtime_error("Attempt to interact with stdin");
        } else if (fd == STDOUT_FILENO) {
            logger->debug("Process interacting with stdout", fd);
        } else if (fd == STDERR_FILENO) {
            logger->debug("Process interacting with stderr", fd);
        } else if (isNotOwned) {
            printf("fd not owned by this thread (%i)\n", fd);
            throw std::runtime_error("fd not owned by this function");
        }
    }

    /** Whitelist specific files to allow open and read-only */
    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_open", I32, __syscall_open, I32 pathPtr, I32 flags, I32 mode) {
        const std::shared_ptr<spdlog::logger> logger = util::getLogger();
        logger->debug("S - open - {} {} {}", pathPtr, flags, mode);

        // Get the path
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        char *path = &Runtime::memoryRef<char>(memoryPtr, (Uptr) pathPtr);

        // Check if this is a valid path. Return a read-only handle to the file if so
        int fd = -1;
        if (strcmp(path, "/etc/hosts") == 0) {
            logger->debug("Opening dummy /etc/hosts");
            fd = open(HOSTS_FILE, 0, 0);

        } else if (strcmp(path, "/etc/resolv.conf") == 0) {
            logger->debug("Opening dummy /etc/resolv.conf");
            fd = open(RESOLV_FILE, 0, 0);

        } else if (strcmp(path, "/dev/urandom") == 0) {
            //TODO avoid use of system-wide urandom
            logger->debug("Opening /dev/urandom");
            fd = open("/dev/urandom", 0, 0);

        } else {
            // Bomb out if not valid path
            printf("Trying to open blocked path (%s) \n", path);
            throw std::runtime_error("Attempt to open invalid file");
        }

        if (fd > 0) {
            openFds.insert(fd);
            return (I32) fd;
        } else if (fd == -ENOENT) {
            printf("File does not exist %s\n", path);
            throw std::runtime_error("File does not exist");
        } else {
            printf("Error opening file %s (code %d) \n", path, fd);
        }

        throw std::runtime_error("Unknown error opening file");
    }

    /** Dummy fcntl implementation, many operations are irrelevant */
    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_fcntl64", I32, __syscall_fcntl64,
                              I32 fd, I32 cmd, I32 c) {
        util::getLogger()->debug("S - fcntl64 - {} {} {}", fd, cmd, c);

        checkThreadOwnsFd(fd);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_read", I32, __syscall_read,
                              I32 fd, I32 bufPtr, I32 count) {
        util::getLogger()->debug("S - read - {} {} {}", fd, bufPtr, count);

        // Provided the thread owns the fd, we allow reading.
        checkThreadOwnsFd(fd);

        // Get the buffer etc.
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        U8 *buf = Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr) bufPtr, (Uptr) count);

        // Do the actual read
        ssize_t bytesRead = read(fd, buf, (size_t) count);

        return (I32) bytesRead;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_close", I32, __syscall_close, I32 fd) {
        util::getLogger()->debug("S - close - {}", fd);

        // Provided the thread owns the fd, we allow closing.
        checkThreadOwnsFd(fd);

        openFds.erase(fd);
        close(fd);

        return 0;
    }

    /** Poll is ok but can pass in an array of structs. */
    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_poll", I32, __syscall_poll, I32 fdsPtr, I32 nfds, I32 timeout) {
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

    DEFINE_INTRINSIC_FUNCTION(env, "ioctl", I32, ioctl, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - ioctl - {} {} {}", a, b, c);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_ioctl", I32, __syscall_ioctl,
                              I32 fd, I32 request, I32 argPtr, I32 d, I32 e, I32 f) {
        util::getLogger()->debug("S - ioctl - {} {} {} {} {} {}", fd, request, argPtr, d, e, f);

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "puts", I32, puts, I32 strPtr) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        char *string = &Runtime::memoryRef<char>(memoryPtr, (Uptr) strPtr);

        util::getLogger()->debug("S - puts", "%s", string);

        return 0;
    }

    struct wasm_iovec {
        U32 iov_base;
        U32 iov_len;
    };

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_writev", I32, __syscall_writev, I32 fd, I32 iov, I32 iovcnt) {
        util::getLogger()->debug("S - writev - {} {} {}", fd, iov, iovcnt);
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;

        // Get array of iovecs from memory
        wasm_iovec *iovecs = Runtime::memoryArrayPtr<wasm_iovec>(memoryPtr, iov, iovcnt);

        // Build vector of emulator iovecs
        iovec nativeIovecs[iovcnt];
        for (U32 i = 0; i < iovcnt; i++) {
            wasm_iovec thisIovec = iovecs[i];

            // Get pointer to data
            U8 *ioData = Runtime::memoryArrayPtr<U8>(memoryPtr, thisIovec.iov_base, thisIovec.iov_len);

            // Create emulator iovec and add to list
            iovec nativeIovec{
                    .iov_base = ioData,
                    .iov_len = thisIovec.iov_len,
            };

            nativeIovecs[i] = nativeIovec;
        }

        Iptr count = writev(fileno(stdout), nativeIovecs, iovcnt);

        return (I32) count;
    }

    // ------------------------
    // I/O - unsupported
    // ------------------------

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_readv", I32, __syscall_readv,
                              I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - readv - {} {} {}", a, b, c);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_llseek", I32, __syscall_llseek, I32 a, I32 b, I32 c, I32 d, I32 e) {
        util::getLogger()->debug("S - llseek - {} {} {} {} {}", a, b, c, d, e);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_futex", I32, __syscall_futex,
                              I32 uaddr, I32 futexOp, I32 val, I32 timeoutPtr, I32 uaddr2, I32 val2) {
        util::getLogger()->debug("S - futex - {} {} {} {} {} {}", uaddr, futexOp, val, timeoutPtr, uaddr2, val2);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_fstat64", I32, __syscall_fstat64, I32 a, I32 b) {
        util::getLogger()->debug("S - fstat64 - {} {}", a, b);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_stat64", I32, __syscall_stat64, I32 a, I32 b) {
        util::getLogger()->debug("S - stat64 - {} {}", a, b);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_access", I32, __syscall_access, I32 a, I32 b) {
        util::getLogger()->debug("S - access - {} {}", a, b);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_chown32", I32, __syscall_chown32, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - chown32 - {} {} {}", a, b, c);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_openat", I32, __syscall_openat, I32 a, I32 b, I32 c, I32 d) {
        util::getLogger()->debug("S - openat - {} {} {} {}", a, b, c, d);

        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_fsync", I32, __syscall_fsync, I32 a) {
        util::getLogger()->debug("S - fsync - {}", a);

        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_dup2", I32, __syscall_dup2, I32 a, I32 b) {
        util::getLogger()->debug("S - dup2 - {} {}", a, b);

        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_dup", I32, __syscall_dup, I32 a) {
        util::getLogger()->debug("S - dup - {}", a);

        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_pipe", I32, __syscall_pipe, I32 a) {
        util::getLogger()->debug("S - pipe - {}", a);

        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall__newselect", I32, __syscall__newselect, I32 a, I32 b, I32 c, I32 d,
                              I32 e) {
        util::getLogger()->debug("S - newselect - {} {} {} {} {}", a, b, c, d, e);

        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_fdatasync", I32, __syscall_fdatasync, I32 a) {
        util::getLogger()->debug("S - fdatasync - {}", a);

        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    // ------------------------
    // Filesystem - unsupported
    // ------------------------

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_creat", I32, __syscall_creat, I32 a, I32 b) {
        util::getLogger()->debug("S - creat - {} {}", a, b);

        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_unlink", I32, __syscall_unlink, I32 a) {
        util::getLogger()->debug("S - unlink - {}", a);

        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_readlink", I32, __syscall_readlink, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - readlink - {} {} {}", a, b, c);

        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_chdir", I32, __syscall_chdir, I32 a) {
        util::getLogger()->debug("S - chdir - {}", a);

        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_umask", I32, __syscall_umask, I32 a) {
        util::getLogger()->debug("S - umask - {}", a);

        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_getdents64", I32, __syscall_getdents64, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - getdents64 - {} {} {}", a, b, c);

        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_lstat64", I32, __syscall_lstat64, I32 a, I32 b) {
        util::getLogger()->debug("S - lstat64 - {} {}", a, b);

        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_rename", I32, __syscall_rename, I32 a, I32 b) {
        util::getLogger()->debug("S - rename - {} {}", a, b);

        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
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

    /** Socket-related structs (see https://beej.us/guide/bgnet/html/multi/sockaddr_inman.html) */
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

    /** Writes changes to a native sockaddr back to a wasm sockaddr. This is important in several
     * networking syscalls that receive responses and modify arguments in place */
    void setSockAddr(sockaddr nativeSockAddr, I32 addrPtr) {
        // Get emulator pointer to wasm address
        wasm_sockaddr *wasmAddrPtr = &Runtime::memoryRef<wasm_sockaddr>(getExecutingModule()->defaultMemory,
                                                                        (Uptr) addrPtr);

        // Modify in place
        wasmAddrPtr->sa_family = nativeSockAddr.sa_family;
        std::copy(nativeSockAddr.sa_data, nativeSockAddr.sa_data + 14, wasmAddrPtr->sa_data);
    }

    void setSockLen(socklen_t nativeValue, I32 wasmPtr) {
        // Get emulator pointer to wasm address
        I32 *wasmAddrPtr = &Runtime::memoryRef<I32>(getExecutingModule()->defaultMemory, (Uptr) wasmPtr);
        std::copy(&nativeValue, &nativeValue + 1, wasmAddrPtr);
    }

    /**
     * When properly isolated, functions will run in their own network namespace,
     * therefore we can be relatively comfortable passing some of the syscalls
     * straight through.
     */
    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_socketcall", I32, __syscall_socketcall, I32 call, I32 argsPtr) {
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

                        // Make the emulator call
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

                throwException(Runtime::Exception::calledUnimplementedIntrinsicType);

            case (SocketCalls::sc_listen): {
                // Server-side
                util::getLogger()->debug("S - listen - {} {}", call, argsPtr);

                throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
            }

            default: {
                printf("Unrecognised socketcall %i\n", call);
                return 0;
            }
        }

        return 0;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "_gethostbyname", I32, _gethostbyname, I32 hostnamePtr) {
        auto hostname = &Runtime::memoryRef<char>(getExecutingModule()->defaultMemory, (Uptr) hostnamePtr);

        util::getLogger()->debug("S - gethostbyname {}", hostname);

        return 0;
    }

    // ------------------------
    // Timing - supported
    // ------------------------

    /** Struct to fake 32-bit time in wasm modules */
    struct wasm_timespec {
        I32 tv_sec;
        I32 tv_nsec;
    };

    //TODO - make this more secure
    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_clock_gettime", I32, __syscall_clock_gettime,
                              I32 clockId, I32 resultAddress) {
        util::getLogger()->debug("S - clock_gettime - {} {}", clockId, resultAddress);

        timespec ts{};
        clock_gettime(clockId, &ts);

        auto result = &Runtime::memoryRef<wasm_timespec>(getExecutingModule()->defaultMemory, (Uptr) resultAddress);
        result->tv_sec = I32(ts.tv_sec);
        result->tv_nsec = I32(ts.tv_nsec);

        return 0;
    }

    // ------------------------
    // Timing - unsupported
    // ------------------------

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_gettimeofday", I32, __syscall_gettimeofday, I32 a, I32 b) {
        util::getLogger()->debug("S - gettimeofday - {} {}", a, b);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_setitimer", I32, __syscall_setitimer, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - setitimer - {} {} {}", a, b, c);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_wait4", I32, __syscall_wait4, I32 a, I32 b, I32 c, I32 d) {
        util::getLogger()->debug("S - wait4 - {} {} {} {}", a, b, c, d);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
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
    // Misc
    // ------------------------

    DEFINE_INTRINSIC_FUNCTION(env, "abort", void, abort) {
        util::getLogger()->debug("S - abort");
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_rt_sigaction", I32, __syscall_rt_sigaction, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - rt_sigaction - {} {} {}", a, b, c);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__funcs_on_exit", void, __funcs_on_exit) {
        util::getLogger()->debug("S - funcs_on_exit", "");
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__libc_exit_fini", void, __libc_exit_fini) {
        util::getLogger()->debug("S - libc_exit_fini", "");
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__stdio_exit", void, __stdio_exit) {
        util::getLogger()->debug("S - stdio_exit", "");
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__unsupported_syscall", I32, __unsupported_syscall,
                              I32 a, I32 b, I32 c, I32 d, I32 e, I32 f, I32 g) {
        util::getLogger()->debug("S - UNSUPPORTED - {} {} {} {} {} {} {}", a, b, c, d, e, f, g);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_gettid", I32, __syscall_gettid, I32 a) {
        util::getLogger()->debug("S - gettid - {}", a);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_getgroups32", I32, __syscall_getgroups32, I32 a, I32 b) {
        util::getLogger()->debug("S - getgroups32 - {} {}", a, b);

        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_tkill", I32, __syscall_tkill, I32 a, I32 b) {
        util::getLogger()->debug("S - tkill - {} {}", a, b);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_kill", I32, __syscall_kill, I32 a, I32 b) {
        util::getLogger()->debug("S - kill - {} {}", a, b);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_tgkill", I32, __syscall_tgkill, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - tgkill - {} {} {}", a, b, c);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_sched_getaffinity", I32, __syscall_sched_getaffinity, I32 a, I32 b,
                              I32 c) {
        util::getLogger()->debug("S - sched_getaffinity - {} {} {}", a, b, c);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_faccessat", I32, __syscall_faccessat, I32 a, I32 b, I32 c, I32 d) {
        util::getLogger()->debug("S - sched_faccessat - {} {} {} {}", a, b, c, d);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_pselect6", I32, __syscall_pselect6, I32 a, I32 b, I32 c, I32 d, I32 e,
                              I32 f) {
        util::getLogger()->debug("S - pselect6 - {} {} {} {} {} {}", a, b, c, d, e, f);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_rt_sigprocmask", I32, __syscall_rt_sigprocmask, I32 a, I32 b, I32 c) {
        util::getLogger()->debug("S - rt_sigprocmask - {} {} {}", a, b, c);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "_Unwind_RaiseException", I32, _Unwind_RaiseException, I32 a) {
        util::getLogger()->debug("S - Unwind_RaiseException - {}", a);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "_Unwind_DeleteException", void, _Unwind_DeleteException, I32 a) {
        util::getLogger()->debug("S - Unwind_DeleteException - {}", a);
        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    // ------------------------
    // Memory - supported
    // ------------------------

    Uptr getNumberPagesAtOffset(U32 offset) {
        // Work out how many pages needed to hit the target address
        Uptr pageCount = ((Uptr) offset) / IR::numBytesPerPage;

        // Check we're on a page boundary, if not bump up number of pages
        if (offset % IR::numBytesPerPage != 0) {
            printf("Warning, requesting address off page boundary (%u)", offset);
            pageCount++;
        }

        return pageCount;
    }

    /**
     * With mmap we will ignore the start address and not support file mapping
     */
    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_mmap", I32, __syscall_mmap,
                              U32 addr, U32 length, U32 prot, U32 flags, I32 fd, U32 offset) {
        util::getLogger()->debug("S - mmap - {} {} {} {} {} {}", addr, length, prot, flags, fd, offset);

        if (addr != 0) {
            printf("Ignoring mmap hint at %i\n", addr);
        }

        if (fd != -1) {
            throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
        }

        // Work out how many pages need to be added
        Uptr pagesRequested = getNumberPagesAtOffset(length);

        Iptr previousPageCount = growMemory(getExecutingModule()->defaultMemory, pagesRequested);

        if (previousPageCount == -1) {
            printf("mmap no memory\n");
            return -ENOMEM;
        }

        // Get pointer to mapped range
        auto mappedRangePtr = (U32) (Uptr(previousPageCount) * IR::numBytesPerPage);
        return mappedRangePtr;
    }

    /**
     * munmap is fairly straightforward, just unmap the relevant pages
     */
    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_munmap", I32, __syscall_munmap,
                              U32 addr, U32 length) {
        util::getLogger()->debug("S - munmap - {} {}", addr, length);

        Runtime::Memory *memory = getExecutingModule()->defaultMemory;

        if (addr & (IR::numBytesPerPage - 1) || length == 0) { return -EINVAL; }

        const Uptr basePageIndex = addr / IR::numBytesPerPage;
        const Uptr numPages = (length + IR::numBytesPerPage - 1) / IR::numBytesPerPage;

        if (basePageIndex + numPages > getMemoryMaxPages(memory)) { return -EINVAL; }

        unmapMemoryPages(memory, basePageIndex, numPages);

        return 0;
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

        Uptr targetPageCount = getNumberPagesAtOffset(addr);

        // Work out current page count and break
        Runtime::Memory *memory = getExecutingModule()->defaultMemory;
        const Uptr currentPageCount = getMemoryNumPages(memory);
        const U32 currentBreak = (U32) ((currentPageCount * IR::numBytesPerPage));

        Uptr maxPages = getMemoryMaxPages(memory);
        if (targetPageCount > maxPages) {
            // Return old break if there's an error
            return currentBreak;
        }

        if (targetPageCount <= currentPageCount || addr == 0) {
            // Return old break if nothing changes or called with zero
            return currentBreak;
        }

        Uptr expansion = targetPageCount - currentPageCount;

        // Grow memory as required
        growMemory(memory, expansion);

        // Return NEW break if successful
        return (U32) addr;
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

        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    // ------------------------
    // Resources - unsupported
    // ------------------------

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_setrlimit", I32, __syscall_setrlimit, I32 a, I32 b) {
        util::getLogger()->debug("S - setrlimit - {} {}", a, b);

        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_getrlimit", I32, __syscall_getrlimit, I32 a, I32 b) {
        util::getLogger()->debug("S - getrlimit - {} {}", a, b);

        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_getrusage", I32, __syscall_getrusage, I32 a, I32 b) {
        util::getLogger()->debug("S - getrusage - {} {}", a, b);

        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_ugetrlimit", I32, __syscall_ugetrlimit, I32 a, I32 b) {
        util::getLogger()->debug("S - ugetrlimit - {} {}", a, b);

        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__syscall_prlimit64", I32, __syscall_prlimit64, I32 a, I32 b, I32 c, I32 d) {
        util::getLogger()->debug("S - prlimit64 - {} {} {} {}", a, b, c, d);

        throwException(Runtime::Exception::calledUnimplementedIntrinsicType);
    }
}