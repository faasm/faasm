#include "WasmModule.h"
#include "syscalls.h"

#include <util/bytes.h>
#include <state/StateKeyValue.h>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/Intrinsics.h>

namespace wasm {
    void faasmLink() {

    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_push_state", void, __faasm_push_state, I32 keyPtr) {
        util::getLogger()->debug("S - push_state - {}", keyPtr);

        auto kv = getStateKV(keyPtr, 0);
        kv->pushFull();
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_push_state_partial", void, __faasm_push_state_partial, I32 keyPtr) {
        util::getLogger()->debug("S - push_state_partial - {}", keyPtr);

        auto kv = getStateKV(keyPtr, 0);
        kv->pushPartial();
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_push_state_partial_mask", void, __faasm_push_state_partial_mask,
            I32 keyPtr, I32 maskKeyPtr) {
        util::getLogger()->debug("S - push_state_partial_mask - {}", keyPtr, maskKeyPtr);

        auto kv = getStateKV(keyPtr, 0);
        auto maskKv = getStateKV(maskKeyPtr, 0);
        kv->pushPartialMask(maskKv);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_pull_state", void, __faasm_pull_state, I32 keyPtr, I32 stateLen) {
        util::getLogger()->debug("S - pull_state - {} {}", keyPtr, stateLen);

        auto kv = getStateKV(keyPtr, stateLen);
        kv->pull();
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_lock_state_read", void, __faasm_lock_state_read, I32 keyPtr) {
        util::getLogger()->debug("S - lock_state_read - {}", keyPtr);

        auto kv = getStateKV(keyPtr, 0);
        kv->lockRead();
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_unlock_state_read", void, __faasm_unlock_state_read, I32 keyPtr) {
        util::getLogger()->debug("S - unlock_state_read - {}", keyPtr);

        auto kv = getStateKV(keyPtr, 0);
        kv->unlockRead();
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_lock_state_write", void, __faasm_lock_state_write, I32 keyPtr) {
        util::getLogger()->debug("S - lock_state_write - {}", keyPtr);

        auto kv = getStateKV(keyPtr, 0);
        kv->lockWrite();
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_unlock_state_write", void, __faasm_unlock_state_write, I32 keyPtr) {
        util::getLogger()->debug("S - unlock_state_write - {}", keyPtr);

        auto kv = getStateKV(keyPtr, 0);
        kv->unlockWrite();
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_write_state", void, __faasm_write_state,
                                   I32 keyPtr, I32 dataPtr, I32 dataLen) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("S - write_state - {} {} {}", keyPtr, dataPtr, dataLen);

        auto kv = getStateKV(keyPtr, dataLen);

        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        U8 *data = Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr) dataPtr, (Uptr) dataLen);

        logger->debug("Writing state length {} to key {}", dataLen, kv->key);
        kv->set(data);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_write_state_offset", void, __faasm_write_state_offset,
                                   I32 keyPtr, I32 totalLen, I32 offset, I32 dataPtr, I32 dataLen) {
        util::getLogger()->debug("S - write_state_offset - {} {} {} {} {}", keyPtr, totalLen, offset, dataPtr,
                                 dataLen);

        auto kv = getStateKV(keyPtr, totalLen);

        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        U8 *data = Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr) dataPtr, (Uptr) dataLen);

        kv->setSegment(offset, data, dataLen);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_read_state", void, __faasm_read_state,
                                   I32 keyPtr, I32 bufferPtr, I32 bufferLen) {
        util::getLogger()->debug("S - read_state - {} {} {}", keyPtr, bufferPtr, bufferLen);

        auto kv = getStateKV(keyPtr, bufferLen);

        // Copy to straight to buffer
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        U8 *buffer = Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr) bufferPtr, (Uptr) bufferLen);
        kv->get(buffer);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_read_state_ptr", I32, __faasm_read_state_ptr,
                                   I32 keyPtr, I32 totalLen) {
        util::getLogger()->debug("S - read_state - {} {}", keyPtr, totalLen);

        auto kv = getStateKV(keyPtr, totalLen);

        // Map shared memory
        WasmModule *module = getExecutingModule();
        U32 wasmPtr = module->mmapKey(kv, totalLen);

        return wasmPtr;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_read_state_offset", void, __faasm_read_state_offset,
                                   I32 keyPtr, I32 totalLen, I32 offset, I32 bufferPtr, I32 bufferLen) {
        util::getLogger()->debug("S - read_state_offset - {} {} {} {} {}", keyPtr, totalLen, offset, bufferPtr,
                                 bufferLen);

        auto kv = getStateKV(keyPtr, totalLen);

        // Copy to straight to buffer
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        U8 *buffer = Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr) bufferPtr, (Uptr) bufferLen);
        kv->getSegment(offset, buffer, bufferLen);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_read_state_offset_ptr", I32, __faasm_read_state_offset_ptr,
                                   I32 keyPtr, I32 totalLen, I32 offset, I32 len) {
        util::getLogger()->debug("S - read_state_offset_ptr - {} {} {} {} {}", keyPtr, totalLen, offset, len);

        auto kv = getStateKV(keyPtr, totalLen);

        // Map whole key in shared memory
        WasmModule *module = getExecutingModule();
        U32 wasmPtr = module->mmapKey(kv, totalLen);

        // Return pointer to offset region
        I32 offsetPtr = wasmPtr + offset;
        return offsetPtr;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_flag_state_dirty", void, __faasm_flag_state_dirty,
                                   I32 keyPtr, I32 totalLen) {
        util::getLogger()->debug("S - __faasm_flag_state_dirty - {} {}", keyPtr, totalLen);

        auto kv = getStateKV(keyPtr, totalLen);
        kv->flagDirty();
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_flag_state_offset_dirty", void, __faasm_flag_state_offset_dirty,
                                   I32 keyPtr, I32 totalLen, I32 offset, I32 len) {
        // Avoid heavy logging
        //        util::getLogger()->debug("S - __faasm_flag_state_offset_dirty - {} {} {} {}", keyPtr, totalLen, offset,
        //                                 len);

        auto kv = getStateKV(keyPtr, totalLen);
        kv->flagSegmentDirty(offset, len);
    }

    I32 _readInputImpl(I32 bufferPtr, I32 bufferLen) {
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

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_read_input", I32, __faasm_read_input, I32 bufferPtr, I32 bufferLen) {
        util::getLogger()->debug("S - read_input - {} {}", bufferPtr, bufferLen);

        return _readInputImpl(bufferPtr, bufferLen);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(tsenv, "__faasm_read_input", I32, __ts_faasm_read_input, I32 bufferPtr,
                                   I32 bufferLen) {
        util::getLogger()->debug("TS - read_input - {} {}", bufferPtr, bufferLen);

        return _readInputImpl(bufferPtr, bufferLen);
    }

    void _writeOutputImpl(I32 outputPtr, I32 outputLen) {
        std::vector<uint8_t> outputData = getBytesFromWasm(outputPtr, outputLen);
        message::Message *call = getExecutingCall();
        call->set_outputdata(outputData.data(), outputData.size());
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_write_output", void, __faasm_write_output, I32 outputPtr,
                                   I32 outputLen) {
        util::getLogger()->debug("S - write_output - {} {}", outputPtr, outputLen);
        _writeOutputImpl(outputPtr, outputLen);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(tsenv, "__faasm_write_output", void, __ts_faasm_write_output, I32 outputPtr,
                                   I32 outputLen) {
        util::getLogger()->debug("TS - write_output - {} {}", outputPtr, outputLen);
        _writeOutputImpl(outputPtr, outputLen);
    }
}