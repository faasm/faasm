#include "WAVMWasmModule.h"
#include "syscalls.h"

#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Runtime.h>

#include <faabric/util/bytes.h>
#include <faabric/util/files.h>
#include <faabric/util/state.h>

using namespace WAVM;

namespace wasm {

void faasmLink() {}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_push_state",
                               void,
                               __faasm_push_state,
                               I32 keyPtr)
{
    auto kv = getStateKV(keyPtr, 0);
    faabric::util::getLogger()->debug("S - push_state - {}", kv->key);
    kv->pushFull();
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_push_state_partial",
                               void,
                               __faasm_push_state_partial,
                               I32 keyPtr)
{
    auto kv = getStateKV(keyPtr, 0);
    faabric::util::getLogger()->debug("S - push_state_partial - {}", kv->key);
    kv->pushPartial();
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_push_state_partial_mask",
                               void,
                               __faasm_push_state_partial_mask,
                               I32 keyPtr,
                               I32 maskKeyPtr)
{
    auto kv = getStateKV(keyPtr, 0);
    faabric::util::getLogger()->debug(
      "S - push_state_partial_mask - {} {}", kv->key, maskKeyPtr);

    auto maskKv = getStateKV(maskKeyPtr, 0);
    kv->pushPartialMask(maskKv);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_pull_state",
                               void,
                               __faasm_pull_state,
                               I32 keyPtr,
                               I32 stateLen)
{
    auto kv = getStateKV(keyPtr, stateLen);
    faabric::util::getLogger()->debug(
      "S - pull_state - {} {}", kv->key, stateLen);

    kv->pull();
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_lock_state_global",
                               void,
                               __faasm_lock_state_global,
                               I32 keyPtr)
{
    auto kv = getStateKV(keyPtr, 0);
    faabric::util::getLogger()->debug("S - lock_state_global - {}", kv->key);

    kv->lockGlobal();
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_unlock_state_global",
                               void,
                               __faasm_unlock_state_global,
                               I32 keyPtr)
{
    auto kv = getStateKV(keyPtr, 0);
    faabric::util::getLogger()->debug("S - unlock_state_global - {}", kv->key);

    kv->unlockGlobal();
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_lock_state_read",
                               void,
                               __faasm_lock_state_read,
                               I32 keyPtr)
{
    auto kv = getStateKV(keyPtr, 0);
    faabric::util::getLogger()->debug("S - lock_state_read - {}", kv->key);

    kv->lockRead();
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_unlock_state_read",
                               void,
                               __faasm_unlock_state_read,
                               I32 keyPtr)
{
    auto kv = getStateKV(keyPtr, 0);
    faabric::util::getLogger()->debug("S - unlock_state_read - {}", kv->key);

    kv->unlockRead();
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_lock_state_write",
                               void,
                               __faasm_lock_state_write,
                               I32 keyPtr)
{
    auto kv = getStateKV(keyPtr, 0);
    faabric::util::getLogger()->debug("S - lock_state_write - {}", kv->key);

    kv->lockWrite();
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_unlock_state_write",
                               void,
                               __faasm_unlock_state_write,
                               I32 keyPtr)
{
    auto kv = getStateKV(keyPtr, 0);
    faabric::util::getLogger()->debug(
      "S - unlock_state_write - {}", keyPtr, kv->key);

    kv->unlockWrite();
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_write_state",
                               void,
                               __faasm_write_state,
                               I32 keyPtr,
                               I32 dataPtr,
                               I32 dataLen)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();

    auto kv = getStateKV(keyPtr, dataLen);
    logger->debug("S - write_state - {} {} {}", kv->key, dataPtr, dataLen);

    Runtime::Memory* memoryPtr = getExecutingWAVMModule()->defaultMemory;
    U8* data =
      Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr)dataPtr, (Uptr)dataLen);

    logger->debug("Writing state length {} to key {}", dataLen, kv->key);
    kv->set(data);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_append_state",
                               void,
                               __faasm_append_state,
                               I32 keyPtr,
                               I32 dataPtr,
                               I32 dataLen)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();

    Runtime::Memory* memoryPtr = getExecutingWAVMModule()->defaultMemory;
    char* key = &Runtime::memoryRef<char>(memoryPtr, (Uptr)keyPtr);
    U8* data =
      Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr)dataPtr, (Uptr)dataLen);

    logger->debug("S - append_state - {} {} {}", key, dataPtr, dataLen);

    auto kv = getStateKV(keyPtr);
    kv->append(data, dataLen);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_read_appended_state",
                               void,
                               __faasm_read_appended_state,
                               I32 keyPtr,
                               I32 bufferPtr,
                               I32 bufferLen,
                               I32 nElems)
{
    Runtime::Memory* memoryPtr = getExecutingWAVMModule()->defaultMemory;
    char* key = &Runtime::memoryRef<char>(memoryPtr, (Uptr)keyPtr);
    U8* buffer =
      Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr)bufferPtr, (Uptr)bufferLen);

    faabric::util::getLogger()->debug("S - read_appended_state - {} {} {} {}",
                                      key,
                                      bufferPtr,
                                      bufferLen,
                                      nElems);

    auto kv = getStateKV(keyPtr, bufferLen);
    kv->getAppended(buffer, bufferLen, nElems);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_clear_appended_state",
                               void,
                               __faasm_clear_appended_state,
                               I32 keyPtr)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();

    Runtime::Memory* memoryPtr = getExecutingWAVMModule()->defaultMemory;
    char* key = &Runtime::memoryRef<char>(memoryPtr, (Uptr)keyPtr);

    logger->debug("S - clear_appended_state - {}", key);
    const std::pair<std::string, std::string> userKey =
      getUserKeyPairFromWasm(keyPtr);

    auto kv = getStateKV(keyPtr);
    kv->clearAppended();
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_write_state_offset",
                               void,
                               __faasm_write_state_offset,
                               I32 keyPtr,
                               I32 totalLen,
                               I32 offset,
                               I32 dataPtr,
                               I32 dataLen)
{
    auto kv = getStateKV(keyPtr, totalLen);
    faabric::util::getLogger()->debug("S - write_state_offset - {} {} {} {} {}",
                                      kv->key,
                                      totalLen,
                                      offset,
                                      dataPtr,
                                      dataLen);

    Runtime::Memory* memoryPtr = getExecutingWAVMModule()->defaultMemory;
    U8* data =
      Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr)dataPtr, (Uptr)dataLen);

    kv->setChunk(offset, data, dataLen);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_write_state_from_file",
                               I32,
                               __faasm_write_state_from_file,
                               I32 keyPtr,
                               I32 pathPtr)
{
    const std::string key = getStringFromWasm(keyPtr);
    const std::string path = getStringFromWasm(pathPtr);

    faabric::util::getLogger()->debug(
      "S - write_state_from_file - {} {}", key, path);

    // Read file into bytes
    const std::string maskedPath = storage::prependRuntimeRoot(path);
    const std::vector<uint8_t> bytes =
      faabric::util::readFileToBytes(maskedPath);
    unsigned long fileLength = bytes.size();

    // Write to state
    auto kv = getStateKV(keyPtr, fileLength);
    kv->set(bytes.data());

    return fileLength;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_read_state",
                               I32,
                               __faasm_read_state,
                               I32 keyPtr,
                               I32 bufferPtr,
                               I32 bufferLen)
{

    // If buffer len is zero, just need the state size
    if (bufferLen == 0) {
        std::string user = getExecutingCall()->user();
        std::string key = getStringFromWasm(keyPtr);
        faabric::util::getLogger()->debug(
          "S - read_state - {} {} {}", key, bufferPtr, bufferLen);

        faabric::state::State& state = faabric::state::getGlobalState();
        return (I32)state.getStateSize(user, key);
    } else {
        auto kv = getStateKV(keyPtr, bufferLen);
        faabric::util::getLogger()->debug(
          "S - read_state - {} {} {}", kv->key, bufferPtr, bufferLen);

        // Copy to straight to buffer
        Runtime::Memory* memoryPtr = getExecutingWAVMModule()->defaultMemory;
        U8* buffer = Runtime::memoryArrayPtr<U8>(
          memoryPtr, (Uptr)bufferPtr, (Uptr)bufferLen);
        kv->get(buffer);
        return kv->size();
    }
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_read_state_ptr",
                               I32,
                               __faasm_read_state_ptr,
                               I32 keyPtr,
                               I32 totalLen)
{
    auto kv = getStateKV(keyPtr, totalLen);
    faabric::util::getLogger()->debug(
      "S - read_state_ptr - {} {}", kv->key, totalLen);

    // Map shared memory
    WAVMWasmModule* module = getExecutingWAVMModule();
    U32 wasmPtr = module->mapSharedStateMemory(kv, 0, totalLen);

    // Call get to make sure the value is pulled
    kv->get();

    return wasmPtr;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_read_state_offset",
                               void,
                               __faasm_read_state_offset,
                               I32 keyPtr,
                               I32 totalLen,
                               I32 offset,
                               I32 bufferPtr,
                               I32 bufferLen)
{
    auto kv = getStateKV(keyPtr, totalLen);
    faabric::util::getLogger()->debug("S - read_state_offset - {} {} {} {} {}",
                                      kv->key,
                                      totalLen,
                                      offset,
                                      bufferPtr,
                                      bufferLen);

    // Copy to straight to buffer
    Runtime::Memory* memoryPtr = getExecutingWAVMModule()->defaultMemory;
    U8* buffer =
      Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr)bufferPtr, (Uptr)bufferLen);
    kv->getChunk(offset, buffer, bufferLen);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_read_state_offset_ptr",
                               I32,
                               __faasm_read_state_offset_ptr,
                               I32 keyPtr,
                               I32 totalLen,
                               I32 offset,
                               I32 len)
{
    auto kv = getStateKV(keyPtr, totalLen);
    faabric::util::getLogger()->debug("S - read_state_offset_ptr - {} {} {} {}",
                                      kv->key,
                                      totalLen,
                                      offset,
                                      len);

    // Map whole key in shared memory
    WAVMWasmModule* module = getExecutingWAVMModule();
    U32 wasmPtr = module->mapSharedStateMemory(kv, offset, len);

    // Call get to make sure the value is there
    kv->getChunk(offset, len);

    return wasmPtr;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_flag_state_dirty",
                               void,
                               __faasm_flag_state_dirty,
                               I32 keyPtr,
                               I32 totalLen)
{
    auto kv = getStateKV(keyPtr, totalLen);
    faabric::util::getLogger()->debug(
      "S - __faasm_flag_state_dirty - {} {}", kv->key, totalLen);

    kv->flagDirty();
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_flag_state_offset_dirty",
                               void,
                               __faasm_flag_state_offset_dirty,
                               I32 keyPtr,
                               I32 totalLen,
                               I32 offset,
                               I32 len)
{
    auto kv = getStateKV(keyPtr, totalLen);

    // Avoid heavy logging
    // faabric::util::getLogger()->debug("S - __faasm_flag_state_offset_dirty -
    // {} {} {} {}", keyPtr, totalLen, offset, len);

    kv->flagChunkDirty(offset, len);
}

I32 _readInputImpl(I32 bufferPtr, I32 bufferLen)
{
    // Get the input
    faabric::Message* call = getExecutingCall();
    std::vector<uint8_t> inputBytes =
      faabric::util::stringToBytes(call->inputdata());

    // If nothing, return nothing
    if (inputBytes.empty()) {
        return 0;
    }

    // Write to the wasm buffer
    Runtime::Memory* memoryPtr = getExecutingWAVMModule()->defaultMemory;
    U8* buffer =
      Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr)bufferPtr, (Uptr)bufferLen);

    int inputSize =
      faabric::util::safeCopyToBuffer(inputBytes, buffer, bufferLen);
    return inputSize;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_read_input",
                               I32,
                               __faasm_read_input,
                               I32 bufferPtr,
                               I32 bufferLen)
{
    faabric::util::getLogger()->debug(
      "S - read_input - {} {}", bufferPtr, bufferLen);

    return _readInputImpl(bufferPtr, bufferLen);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(tsenv,
                               "__faasm_read_input",
                               I32,
                               __ts_faasm_read_input,
                               I32 bufferPtr,
                               I32 bufferLen)
{
    faabric::util::getLogger()->debug(
      "TS - read_input - {} {}", bufferPtr, bufferLen);

    return _readInputImpl(bufferPtr, bufferLen);
}

void _writeOutputImpl(I32 outputPtr, I32 outputLen)
{
    std::vector<uint8_t> outputData = getBytesFromWasm(outputPtr, outputLen);
    faabric::Message* call = getExecutingCall();
    call->set_outputdata(outputData.data(), outputData.size());
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_write_output",
                               void,
                               __faasm_write_output,
                               I32 outputPtr,
                               I32 outputLen)
{
    faabric::util::getLogger()->debug(
      "S - write_output - {} {}", outputPtr, outputLen);
    _writeOutputImpl(outputPtr, outputLen);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(tsenv,
                               "__faasm_write_output",
                               void,
                               __ts_faasm_write_output,
                               I32 outputPtr,
                               I32 outputLen)
{
    faabric::util::getLogger()->debug(
      "TS - write_output - {} {}", outputPtr, outputLen);
    _writeOutputImpl(outputPtr, outputLen);
}

void _readPythonInput(I32 buffPtr, I32 buffLen, const std::string& value)
{
    // Get wasm buffer
    U8* buffer = Runtime::memoryArrayPtr<U8>(
      getExecutingWAVMModule()->defaultMemory, (Uptr)buffPtr, (Uptr)buffLen);

    if (value.empty()) {
        // If nothing, just write a null terminator
        buffer[0] = '\0';
    } else {
        // Copy value into WASM
        std::vector<uint8_t> bytes = faabric::util::stringToBytes(value);
        std::copy(bytes.begin(), bytes.end(), buffer);

        // Add null terminator
        buffer[value.size()] = '\0';
    }
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_get_py_user",
                               void,
                               __faasm_get_py_user,
                               I32 bufferPtr,
                               I32 bufferLen)
{
    faabric::util::getLogger()->debug(
      "S - get_py_user - {} {}", bufferPtr, bufferLen);
    std::string value = getExecutingCall()->pythonuser();
    _readPythonInput(bufferPtr, bufferLen, value);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_get_py_func",
                               void,
                               __faasm_get_py_func,
                               I32 bufferPtr,
                               I32 bufferLen)
{
    faabric::util::getLogger()->debug(
      "S - get_py_func - {} {}", bufferPtr, bufferLen);
    std::string value = getExecutingCall()->pythonfunction();
    _readPythonInput(bufferPtr, bufferLen, value);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_get_py_entry",
                               void,
                               __faasm_get_py_entry,
                               I32 bufferPtr,
                               I32 bufferLen)
{
    faabric::util::getLogger()->debug(
      "S - get_py_entry - {} {}", bufferPtr, bufferLen);
    std::string value = getExecutingCall()->pythonentry();
    _readPythonInput(bufferPtr, bufferLen, value);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_conf_flag",
                               U32,
                               __faasm_conf_flag,
                               I32 keyPtr)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();
    const std::string key = getStringFromWasm(keyPtr);
    logger->debug("S - conf_flag - {}", key);

    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();

    if (key == "PYTHON_PRELOAD") {
        int res = conf.pythonPreload == "on" ? 1 : 0;
        return res;
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
}
