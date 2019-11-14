#include "WasmModule.h"
#include "syscalls.h"

#include <util/bytes.h>
#include <state/StateKeyValue.h>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <redis/Redis.h>
#include <util/state.h>
#include <util/files.h>

namespace wasm {
    void faasmLink() {

    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_push_state", void, __faasm_push_state, I32 keyPtr) {
        auto kv = getStateKV(keyPtr, 0);
        util::getLogger()->debug("S - push_state - {}", kv->key);
        kv->pushFull();
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_push_state_partial", void, __faasm_push_state_partial, I32 keyPtr) {
        auto kv = getStateKV(keyPtr, 0);
        util::getLogger()->debug("S - push_state_partial - {}", kv->key);
        kv->pushPartial();
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_push_state_partial_mask", void, __faasm_push_state_partial_mask,
                                   I32 keyPtr, I32 maskKeyPtr) {
        auto kv = getStateKV(keyPtr, 0);
        util::getLogger()->debug("S - push_state_partial_mask - {} {}", kv->key, maskKeyPtr);

        auto maskKv = getStateKV(maskKeyPtr, 0);
        kv->pushPartialMask(maskKv);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_pull_state", void, __faasm_pull_state, I32 keyPtr, I32 stateLen) {
        auto kv = getStateKV(keyPtr, stateLen);
        util::getLogger()->debug("S - pull_state - {} {}", kv->key, stateLen);

        kv->pull();
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_lock_state_read", void, __faasm_lock_state_read, I32 keyPtr) {
        auto kv = getStateKV(keyPtr, 0);
        util::getLogger()->debug("S - lock_state_read - {}", kv->key);

        kv->lockRead();
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_unlock_state_read", void, __faasm_unlock_state_read, I32 keyPtr) {
        auto kv = getStateKV(keyPtr, 0);
        util::getLogger()->debug("S - unlock_state_read - {}", kv->key);

        kv->unlockRead();
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_lock_state_write", void, __faasm_lock_state_write, I32 keyPtr) {
        auto kv = getStateKV(keyPtr, 0);
        util::getLogger()->debug("S - lock_state_write - {}", kv->key);

        kv->lockWrite();
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_unlock_state_write", void, __faasm_unlock_state_write, I32 keyPtr) {
        auto kv = getStateKV(keyPtr, 0);
        util::getLogger()->debug("S - unlock_state_write - {}", keyPtr, kv->key);

        kv->unlockWrite();
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_write_state", void, __faasm_write_state,
                                   I32 keyPtr, I32 dataPtr, I32 dataLen) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        auto kv = getStateKV(keyPtr, dataLen);
        logger->debug("S - write_state - {} {} {}", kv->key, dataPtr, dataLen);

        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        U8 *data = Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr) dataPtr, (Uptr) dataLen);

        logger->debug("Writing state length {} to key {}", dataLen, kv->key);
        kv->set(data);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_append_state", void, __faasm_append_state,
                                   I32 keyPtr, I32 dataPtr, I32 dataLen) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        char *key = &Runtime::memoryRef<char>(memoryPtr, (Uptr) keyPtr);
        const std::string actualKey = util::keyForUser(getExecutingCall()->user(), key);

        logger->debug("S - append_state - {} {} {}", actualKey, dataPtr, dataLen);

        U8 *data = Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr) dataPtr, (Uptr) dataLen);
        std::vector<uint8_t> bytes(data, data + dataLen);

        redis::Redis &redis = redis::Redis::getState();
        redis.enqueueBytes(actualKey, bytes);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_read_appended_state", void, __faasm_read_appended_state,
                                   I32 keyPtr, I32 bufferPtr, I32 bufferLen, I32 nElems) {
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        char *key = &Runtime::memoryRef<char>(memoryPtr, (Uptr) keyPtr);
        const std::string actualKey = util::keyForUser(getExecutingCall()->user(), key);

        util::getLogger()->debug("S - read_appended_state - {} {} {} {}", actualKey, bufferPtr, bufferLen, nElems);

        // Dequeue straight to buffer
        redis::Redis &redis = redis::Redis::getState();

        U8 *buffer = Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr) bufferPtr, (Uptr) bufferLen);
        redis.dequeueMultiple(actualKey, buffer, bufferLen, nElems);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_clear_appended_state", void, __faasm_clear_appended_state,
                                   I32 keyPtr) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        char *key = &Runtime::memoryRef<char>(memoryPtr, (Uptr) keyPtr);
        const std::string actualKey = util::keyForUser(getExecutingCall()->user(), key);

        logger->debug("S - clear_appended_state - {}", actualKey);

        redis::Redis &redis = redis::Redis::getState();
        redis.del(actualKey);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_write_state_offset", void, __faasm_write_state_offset,
                                   I32 keyPtr, I32 totalLen, I32 offset, I32 dataPtr, I32 dataLen) {
        auto kv = getStateKV(keyPtr, totalLen);
        util::getLogger()->debug("S - write_state_offset - {} {} {} {} {}", kv->key, totalLen, offset, dataPtr,
                                 dataLen);

        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        U8 *data = Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr) dataPtr, (Uptr) dataLen);

        kv->setSegment(offset, data, dataLen);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_write_state_from_file", void, __faasm_write_state_from_file,
                                   I32 keyPtr, I32 pathPtr) {
        const std::string key = getStringFromWasm(keyPtr);
        const std::string path = getStringFromWasm(pathPtr);

        util::getLogger()->debug("S - write_state_from_file - {} {}", key, path);

        // Read file into bytes
        const std::vector<uint8_t> bytes = util::readFileToBytes(path);

        // Write to state
        const std::string actualKey = util::keyForUser(getExecutingCall()->user(), key);
        redis::Redis &redis = redis::Redis::getState();
        redis.set(actualKey, bytes);
    }     
    
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_read_state", void, __faasm_read_state,
                                   I32 keyPtr, I32 bufferPtr, I32 bufferLen) {
        auto kv = getStateKV(keyPtr, bufferLen);
        util::getLogger()->debug("S - read_state - {} {} {}", kv->key, bufferPtr, bufferLen);

        // Copy to straight to buffer
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        U8 *buffer = Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr) bufferPtr, (Uptr) bufferLen);
        kv->get(buffer);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_read_state_ptr", I32, __faasm_read_state_ptr,
                                   I32 keyPtr, I32 totalLen) {
        auto kv = getStateKV(keyPtr, totalLen);
        util::getLogger()->debug("S - read_state - {} {}", kv->key, totalLen);

        // Map shared memory
        WasmModule *module = getExecutingModule();
        U32 wasmPtr = module->mmapKey(kv, 0, totalLen);

        return wasmPtr;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_read_state_offset", void, __faasm_read_state_offset,
                                   I32 keyPtr, I32 totalLen, I32 offset, I32 bufferPtr, I32 bufferLen) {
        auto kv = getStateKV(keyPtr, totalLen);
        util::getLogger()->debug("S - read_state_offset - {} {} {} {} {}", kv->key, totalLen, offset, bufferPtr,
                                 bufferLen);

        // Copy to straight to buffer
        Runtime::Memory *memoryPtr = getExecutingModule()->defaultMemory;
        U8 *buffer = Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr) bufferPtr, (Uptr) bufferLen);
        kv->getSegment(offset, buffer, bufferLen);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_read_state_offset_ptr", I32, __faasm_read_state_offset_ptr,
                                   I32 keyPtr, I32 totalLen, I32 offset, I32 len) {
        auto kv = getStateKV(keyPtr, totalLen);
        util::getLogger()->debug("S - read_state_offset_ptr - {} {} {} {}", kv->key, totalLen, offset, len);

        // Map whole key in shared memory
        WasmModule *module = getExecutingModule();
        U32 wasmPtr = module->mmapKey(kv, offset, len);
        return wasmPtr;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_flag_state_dirty", void, __faasm_flag_state_dirty,
                                   I32 keyPtr, I32 totalLen) {
        auto kv = getStateKV(keyPtr, totalLen);
        util::getLogger()->debug("S - __faasm_flag_state_dirty - {} {}", kv->key, totalLen);

        kv->flagDirty();
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_flag_state_offset_dirty", void, __faasm_flag_state_offset_dirty,
                                   I32 keyPtr, I32 totalLen, I32 offset, I32 len) {
        auto kv = getStateKV(keyPtr, totalLen);
        // Avoid heavy logging
        //        util::getLogger()->debug("S - __faasm_flag_state_offset_dirty - {} {} {} {}", keyPtr, totalLen, offset,
        //                                 len);

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

    void _readPythonInput(I32 buffPtr, I32 buffLen, const std::string &value) {
        // If nothing ignore
        if (value.empty()) {
            return;
        }

        // Copy value into WASM
        U8 *buffer = Runtime::memoryArrayPtr<U8>(getExecutingModule()->defaultMemory, (Uptr) buffPtr, (Uptr) buffLen);
        std::vector<uint8_t> bytes = util::stringToBytes(value);
        std::copy(bytes.begin(), bytes.end(), buffer);

        // Add null terminator
        buffer[value.size()] = '\0';
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_get_py_user", void, __faasm_get_py_user, I32 bufferPtr,
                                   I32 bufferLen) {
        util::getLogger()->debug("S - get_py_user - {} {}", bufferPtr, bufferLen);
        std::string value = getExecutingCall()->pythonuser();
        _readPythonInput(bufferPtr, bufferLen, value);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_get_py_func", void, __faasm_get_py_func, I32 bufferPtr,
                                   I32 bufferLen) {
        util::getLogger()->debug("S - get_py_func - {} {}", bufferPtr, bufferLen);
        std::string value = getExecutingCall()->pythonfunction();
        _readPythonInput(bufferPtr, bufferLen, value);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "__faasm_conf_flag", U32, __faasm_conf_flag, I32 keyPtr) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        const std::string key = getStringFromWasm(keyPtr);
        logger->debug("S - conf_flag - {}", key);

        util::SystemConfig &conf = util::getSystemConfig();

        if (key == "PRELOAD_NUMPY") {
            return conf.pythonPreload == "on";
        } else if (key == "ALWAYS_ON") {
            // For testing
            return 1;
        } else if (key == "ALWAYS_OFF") {
            // For testing
            return 0;
        } else {
            logger->warn("Unknown conf flag: {}", key);
            return 0;
        }
    }

    // Emulator API, should not be called from wasm but needs to be present for linking
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "setEmulatedMessageFromJson", I32, setEmulatedMessageFromJson, I32 msgPtr) {
        util::getLogger()->debug("S - setEmulatedMessageFromJson - {}", msgPtr);
        throw std::runtime_error("Should not be calling emulator functions from wasm");
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "emulatorGetAsyncResponse", I32, emulatorGetAsyncResponse) {
        util::getLogger()->debug("S - emulatorGetAsyncResponse");
        throw std::runtime_error("Should not be calling emulator functions from wasm");
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "emulatorSetCallStatus", void, emulatorSetCallStatus, I32 success) {
        util::getLogger()->debug("S - emulatorSetCallStatus {}", success);
        throw std::runtime_error("Should not be calling emulator functions from wasm");
    }
}