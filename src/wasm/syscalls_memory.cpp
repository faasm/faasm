#include "WasmModule.h"
#include "syscalls.h"

#include <util/bytes.h>
#include <linux/membarrier.h>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/RuntimeData.h>
#include <WAVM/Runtime/Intrinsics.h>

namespace wasm {
    I32 s__madvise(I32 address, I32 numBytes, I32 advice) {
        util::getLogger()->debug("S - madvise - {} {} {}", address, numBytes, advice);

        return 0;
    }

    I32 s__membarrier(I32 a) {
        util::getLogger()->debug("S - membarrier - {}", a);

        int res;
        if (a == MEMBARRIER_CMD_QUERY) {
            res = syscall(__NR_membarrier, MEMBARRIER_CMD_QUERY, 0);
        } else if (a == MEMBARRIER_CMD_SHARED || a == MEMBARRIER_CMD_PRIVATE_EXPEDITED ||
                   a == MEMBARRIER_CMD_REGISTER_PRIVATE_EXPEDITED) {
            // We can ignore all non-query membarrier operations
            res = 0;
        } else {
            util::getLogger()->error("Unexpected membarrier argument {}", a);
            throw std::runtime_error("Invalid membarrier command");
        }

        return res;
    }

    I32 s__sigaltstack(I32 ssPtr, I32 oldSsPtr) {
        util::getLogger()->debug("S - sigaltstack - {} {}", ssPtr, oldSsPtr);

        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;

        // Extract struct pointers from wasm memory
        // wasm_stack_t *ss = &Runtime::memoryRef<wasm_stack_t>(memoryPtr, (Uptr) ssPtr);
        wasm_stack_t *oldSs = &Runtime::memoryRef<wasm_stack_t>(memoryPtr, (Uptr) oldSsPtr);

        // Say that the old sigaltstack was disabled and null
        oldSs->ss_sp = 0;
        oldSs->ss_flags = SS_DISABLE;

        return 0;
    }

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

    /**
     * Note that syscall 192 is mmap2, which has the same interface as mmap except that the final argument specifies
     * the offset into the file in 4096-byte units (instead of bytes, as is done by mmap). Given that we
     * ignore the offset we can just treat it like mmap
     */
    I32 s__mmap(I32 addr, I32 length, I32 prot, I32 flags, I32 fd, I32 offset) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - mmap - {} {} {} {} {} {}", addr, length, prot, flags, fd, offset);

        // Although we are ignoring the offset we should probably
        // double check when something explicitly requests one
        if(offset != 0) {
            logger->warn("WARNING: ignoring non-zero mmap offset ({})", offset);
        }

        // Likewise with the address hint
        if (addr != 0) {
            logger->warn("WARNING: ignoring mmap hint at {}", addr);
        }

        // Don't allow using mmap for anything other than allocating memory
        if (fd != -1) {
            throw std::runtime_error("Attempted to mmap file descriptor");
        }

        WasmModule *module = getExecutingModule();

        return module->mmap(length);
    }

    /**
     * Note - the return value required from munmap in case of error is not clear.
     * To be safe this both sets errno and returns the (-1 * error code) (rather than
     * either just returning the error code or returning -1 and setting errno)
     */
    I32 s__munmap(I32 addr, I32 length) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - munmap - {} {}", addr, length);

        WasmModule *executingModule = getExecutingModule();
        Runtime::Memory *memory = executingModule->defaultMemory;

        // If not aligned or zero length, drop out
        Uptr addrPtr = (Uptr) addr;
        if (addrPtr & (IR::numBytesPerPage - 1)) {
            logger->warn("munmap address not page-aligned ({})", addrPtr);
            executingModule->setErrno(EINVAL);
            return -EINVAL;
        } else if(length == 0) {
            logger->warn("munmap size zero");
            executingModule->setErrno(EINVAL);
            return -EINVAL;
        }

        const Uptr addrPageBase = addr / IR::numBytesPerPage;
        const Uptr numPages = getNumberOfPagesForBytes(length);

        // Drop out if we're munmapping over the max page boundary
        if (addrPageBase + numPages > getMemoryMaxPages(memory)) {
            logger->warn("munmapping region over max memory pages");
            executingModule->setErrno(EINVAL);
            return -EINVAL;
        }

        // Unmap the memory
        unmapMemoryPages(memory, addrPageBase, numPages);

        return 0;
    }

    /**
   * Details of what should be returned from brk aren't particularly clear. I took info from
   * https://linux.die.net/man/2/brk i.e.
   *
   *   - on addr == 0 return current break
   *   - on error return current break
   *   - on success return NEW break
   */
    I32 s__brk(I32 addr) {
        util::getLogger()->debug("S - brk - {}", addr);

        Runtime::Memory *memory = getExecutingModule()->defaultMemory;
        const U32 currentBreak = (U32) ((getMemoryNumPages(memory) * IR::numBytesPerPage));

        // Attempt the expansion
        int expandRes = getExecutingModule()->brk(currentBreak + addr);

        if (expandRes == EXPAND_TOO_BIG) {
            return currentBreak;
        } else if (expandRes == EXPAND_NO_ACTION) {
            return currentBreak;
        } else {
            // Success, return requested break
            return (U32) addr;
        }
    }

    DEFINE_INTRINSIC_FUNCTION(env, "sbrk", I32, sbrk, I32 increment) {
        util::getLogger()->debug("S - sbrk - {}", increment);

        WasmModule *module = getExecutingModule();
        return module->sBrk(increment);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_push_state", void, __faasm_push_state, I32 keyPtr) {
        util::getLogger()->debug("S - push_state - {}", keyPtr);

        auto kv = getStateKV(keyPtr, 0);
        kv->pushFull();
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_push_state_partial", void, __faasm_push_state_partial, I32 keyPtr) {
        util::getLogger()->debug("S - push_state_partial - {}", keyPtr);

        auto kv = getStateKV(keyPtr, 0);
        kv->pushPartial();
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_lock_state_read", void, __faasm_lock_state_read, I32 keyPtr) {
        util::getLogger()->debug("S - lock_state_read - {}", keyPtr);

        auto kv = getStateKV(keyPtr, 0);
        kv->lockRead();
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_unlock_state_read", void, __faasm_unlock_state_read, I32 keyPtr) {
        util::getLogger()->debug("S - unlock_state_read - {}", keyPtr);

        auto kv = getStateKV(keyPtr, 0);
        kv->unlockRead();
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_lock_state_write", void, __faasm_lock_state_write, I32 keyPtr) {
        util::getLogger()->debug("S - lock_state_write - {}", keyPtr);

        auto kv = getStateKV(keyPtr, 0);
        kv->lockWrite();
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_unlock_state_write", void, __faasm_unlock_state_write, I32 keyPtr) {
        util::getLogger()->debug("S - unlock_state_write - {}", keyPtr);

        auto kv = getStateKV(keyPtr, 0);
        kv->unlockWrite();
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_write_state", void, __faasm_write_state,
                              I32 keyPtr, I32 dataPtr, I32 dataLen, I32 async) {
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

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_write_state_offset", void, __faasm_write_state_offset,
                              I32 keyPtr, I32 totalLen, I32 offset, I32 dataPtr, I32 dataLen, I32 async) {
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

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_read_state", void, __faasm_read_state,
                              I32 keyPtr, I32 bufferPtr, I32 bufferLen, I32 async) {
        util::getLogger()->debug("S - read_state - {} {} {} {}", keyPtr, bufferPtr, bufferLen, async);

        auto kv = getStateKVRead(keyPtr, bufferLen, async);

        // Copy to straight to buffer
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        U8 *buffer = Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr) bufferPtr, (Uptr) bufferLen);
        kv->get(buffer);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_read_state_ptr", I32, __faasm_read_state_ptr,
                              I32 keyPtr, I32 totalLen, I32 async) {
        util::getLogger()->debug("S - read_state - {} {} {}", keyPtr, totalLen, async);

        auto kv = getStateKVRead(keyPtr, totalLen, async);

        // Map shared memory
        WasmModule *module = getExecutingModule();
        U32 wasmPtr = module->mmapKey(kv, totalLen);

        return wasmPtr;
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_read_state_offset", void, __faasm_read_state_offset,
                              I32 keyPtr, I32 totalLen, I32 offset, I32 bufferPtr, I32 bufferLen, I32 async) {
        util::getLogger()->debug("S - read_state_offset - {} {} {} {} {}", keyPtr, totalLen, offset, bufferPtr,
                                 bufferLen);

        auto kv = getStateKVRead(keyPtr, totalLen, async);

        // Copy to straight to buffer
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        U8 *buffer = Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr) bufferPtr, (Uptr) bufferLen);
        kv->getSegment(offset, buffer, bufferLen);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_read_state_offset_ptr", I32, __faasm_read_state_offset_ptr,
                              I32 keyPtr, I32 totalLen, I32 offset, I32 len, I32 async) {
        util::getLogger()->debug("S - read_state_offset_ptr - {} {} {} {} {}", keyPtr, totalLen, offset, len, async);

        auto kv = getStateKVRead(keyPtr, totalLen, async);

        // Map whole key in shared memory
        WasmModule *module = getExecutingModule();
        U32 wasmPtr = module->mmapKey(kv, totalLen);

        // Return pointer to offset region
        I32 offsetPtr = wasmPtr + offset;
        return offsetPtr;
    }


    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_flag_state_dirty", void, __faasm_flag_state_dirty,
                              I32 keyPtr, I32 totalLen) {
        util::getLogger()->debug("S - __faasm_flag_state_dirty - {} {}", keyPtr, totalLen);

        auto kv = getStateKV(keyPtr, totalLen);
        kv->flagFullValueDirty();
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_flag_state_offset_dirty", void, __faasm_flag_state_offset_dirty,
                              I32 keyPtr, I32 totalLen, I32 offset, I32 len) {
        // Avoid heavy logging
        //        util::getLogger()->debug("S - __faasm_flag_state_offset_dirty - {} {} {} {}", keyPtr, totalLen, offset,
        //                                 len);

        auto kv = getStateKV(keyPtr, totalLen);
        kv->flagSegmentDirty(offset, len);
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_read_input", I32, __faasm_read_input, I32 bufferPtr, I32 bufferLen) {
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

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_snapshot_memory", void, __faasm_snapshot_memory, I32 keyPtr) {
        util::getLogger()->debug("S - snapshot_memory - {}", keyPtr);
        WasmModule *module = getExecutingModule();

        std::string key = getStringFromWasm(keyPtr);
        module->snapshotFullMemory(key.c_str());
    }

    DEFINE_INTRINSIC_FUNCTION(env, "__faasm_restore_memory", void, __faasm_restore_memory, I32 keyPtr) {
        util::getLogger()->debug("S - restore_memory - {}", keyPtr);

        WasmModule *module = getExecutingModule();

        std::string key = getStringFromWasm(keyPtr);
        module->restoreFullMemory(key.c_str());
    }
}