#include "syscalls.h"

#include <conf/FaasmConfig.h>
#include <faabric/executor/ExecutorContext.h>
#include <faabric/snapshot/SnapshotClient.h>
#include <faabric/snapshot/SnapshotRegistry.h>
#include <faabric/util/bytes.h>
#include <faabric/util/files.h>
#include <faabric/util/logging.h>
#include <faabric/util/macros.h>
#include <faabric/util/state.h>
#include <wasm/chaining.h>
#include <wasm/host_interface_test.h>
#include <wasm/migration.h>
#include <wasm/faasm.h>
#include <wavm/WAVMWasmModule.h>

#include <WAVM/Platform/Diagnostics.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Runtime.h>

using namespace WAVM;
using namespace faabric::transport;
using namespace faabric::executor;

namespace wasm {

void faasmLink() {}

// ------------------------------------
// STATE
// ------------------------------------

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_push_state",
                               void,
                               __faasm_push_state,
                               I32 keyPtr)
{
    auto kv = getStateKV(keyPtr, 0);
    SPDLOG_DEBUG("S - push_state - {}", kv->key);
    kv->pushFull();
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_push_state_partial",
                               void,
                               __faasm_push_state_partial,
                               I32 keyPtr)
{
    auto kv = getStateKV(keyPtr, 0);
    SPDLOG_DEBUG("S - push_state_partial - {}", kv->key);
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
    SPDLOG_DEBUG("S - push_state_partial_mask - {} {}", kv->key, maskKeyPtr);

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
    SPDLOG_DEBUG("S - pull_state - {} {}", kv->key, stateLen);

    kv->pull();
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_lock_state_read",
                               void,
                               __faasm_lock_state_read,
                               I32 keyPtr)
{
    auto kv = getStateKV(keyPtr, 0);
    SPDLOG_DEBUG("S - lock_state_read - {}", kv->key);

    kv->lockRead();
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_unlock_state_read",
                               void,
                               __faasm_unlock_state_read,
                               I32 keyPtr)
{
    auto kv = getStateKV(keyPtr, 0);
    SPDLOG_DEBUG("S - unlock_state_read - {}", kv->key);

    kv->unlockRead();
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_lock_state_write",
                               void,
                               __faasm_lock_state_write,
                               I32 keyPtr)
{
    auto kv = getStateKV(keyPtr, 0);
    SPDLOG_DEBUG("S - lock_state_write - {}", kv->key);

    kv->lockWrite();
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_unlock_state_write",
                               void,
                               __faasm_unlock_state_write,
                               I32 keyPtr)
{
    auto kv = getStateKV(keyPtr, 0);
    SPDLOG_DEBUG("S - unlock_state_write - {}", keyPtr, kv->key);

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

    auto kv = getStateKV(keyPtr, dataLen);
    SPDLOG_DEBUG("S - write_state - {} {} {}", kv->key, dataPtr, dataLen);

    Runtime::Memory* memoryPtr = getExecutingWAVMModule()->defaultMemory;
    U8* data =
      Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr)dataPtr, (Uptr)dataLen);

    SPDLOG_DEBUG("Writing state length {} to key {}", dataLen, kv->key);
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
    Runtime::Memory* memoryPtr = getExecutingWAVMModule()->defaultMemory;
    U8* data =
      Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr)dataPtr, (Uptr)dataLen);

    SPDLOG_DEBUG("S - append_state - {} {} {}",
                 &Runtime::memoryRef<char>(memoryPtr, (Uptr)keyPtr),
                 dataPtr,
                 dataLen);

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
    U8* buffer =
      Runtime::memoryArrayPtr<U8>(memoryPtr, (Uptr)bufferPtr, (Uptr)bufferLen);

    SPDLOG_DEBUG("S - read_appended_state - {} {} {} {}",
                 &Runtime::memoryRef<char>(memoryPtr, (Uptr)keyPtr),
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

    Runtime::Memory* memoryPtr = getExecutingWAVMModule()->defaultMemory;
    UNUSED(memoryPtr);

    SPDLOG_DEBUG("S - clear_appended_state - {}",
                 &Runtime::memoryRef<char>(memoryPtr, (Uptr)keyPtr));

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
    SPDLOG_DEBUG("S - write_state_offset - {} {} {} {} {}",
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

    SPDLOG_DEBUG("S - write_state_from_file - {} {}", key, path);

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
        std::string user = ExecutorContext::get()->getMsg().user();
        std::string key = getStringFromWasm(keyPtr);
        SPDLOG_DEBUG("S - read_state - {} {} {}", key, bufferPtr, bufferLen);

        faabric::state::State& state = faabric::state::getGlobalState();
        return (I32)state.getStateSize(user, key);
    } else {
        auto kv = getStateKV(keyPtr, bufferLen);
        SPDLOG_DEBUG(
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
    SPDLOG_DEBUG("S - read_state_ptr - {} {}", kv->key, totalLen);

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
    SPDLOG_DEBUG("S - read_state_offset - {} {} {} {} {}",
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
    SPDLOG_DEBUG("S - read_state_offset_ptr - {} {} {} {}",
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
    SPDLOG_DEBUG("S - __faasm_flag_state_dirty - {} {}", kv->key, totalLen);

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
    // SPDLOG_DEBUG("S - __faasm_flag_state_offset_dirty -
    // {} {} {} {}", keyPtr, totalLen, offset, len);

    kv->flagChunkDirty(offset, len);
}

I32 _readInputImpl(I32 bufferPtr, I32 bufferLen)
{
    // Get the input
    faabric::Message* call = &ExecutorContext::get()->getMsg();
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

// ------------------------------------
// FUNCTIONS
// ------------------------------------

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_read_input",
                               I32,
                               __faasm_read_input,
                               I32 bufferPtr,
                               I32 bufferLen)
{
    SPDLOG_DEBUG("S - read_input - {} {}", bufferPtr, bufferLen);

    return _readInputImpl(bufferPtr, bufferLen);
}

void _writeOutputImpl(I32 outputPtr, I32 outputLen)
{
    std::string outputData = getStringFromWasm(outputPtr);
    faabric::Message* call = &ExecutorContext::get()->getMsg();
    call->set_outputdata(outputData);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_write_output",
                               void,
                               __faasm_write_output,
                               I32 outputPtr,
                               I32 outputLen)
{
    SPDLOG_DEBUG("S - write_output - {} {}", outputPtr, outputLen);
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
    SPDLOG_DEBUG("S - get_py_user - {} {}", bufferPtr, bufferLen);
    std::string value = ExecutorContext::get()->getMsg().pythonuser();

    if (value.empty()) {
        throw std::runtime_error("Python user empty, cannot return");
    }

    _readPythonInput(bufferPtr, bufferLen, value);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_get_py_func",
                               void,
                               __faasm_get_py_func,
                               I32 bufferPtr,
                               I32 bufferLen)
{
    SPDLOG_DEBUG("S - get_py_func - {} {}", bufferPtr, bufferLen);
    std::string value = ExecutorContext::get()->getMsg().pythonfunction();
    _readPythonInput(bufferPtr, bufferLen, value);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_get_py_entry",
                               void,
                               __faasm_get_py_entry,
                               I32 bufferPtr,
                               I32 bufferLen)
{
    SPDLOG_DEBUG("S - get_py_entry - {} {}", bufferPtr, bufferLen);
    std::string value = ExecutorContext::get()->getMsg().pythonentry();
    _readPythonInput(bufferPtr, bufferLen, value);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_conf_flag",
                               U32,
                               __faasm_conf_flag,
                               I32 keyPtr)
{

    const std::string key = getStringFromWasm(keyPtr);
    SPDLOG_DEBUG("S - conf_flag - {}", key);

    conf::FaasmConfig& conf = conf::getFaasmConfig();
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
        SPDLOG_WARN("Unknown conf flag: {}", key);
        return 0;
    }
}

// ------------------------------------
// DEBUGGING
// ------------------------------------

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_backtrace",
                               void,
                               __faasm_backtrace,
                               I32 depth)
{

    SPDLOG_DEBUG("S - faasm_backtrace {}", depth);

    WAVMWasmModule* module = getExecutingWAVMModule();
    module->printDebugInfo();

    Platform::CallStack callStack = Platform::captureCallStack(depth);
    std::vector<std::string> callStackStrs =
      Runtime::describeCallStack(callStack);

    printf("\n------ Faasm backtrace ------\n");
    for (auto s : callStackStrs) {
        printf("%s\n", s.c_str());
    }
    printf("-------------------------------\n");

    fflush(stdout);
}

// ------------------------------------
// SHARED MEMORY
// ------------------------------------

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_sm_reduce",
                               void,
                               __faasm_sm_reduce,
                               I32 varPtr,
                               I32 varType,
                               I32 reduceOp,
                               int currentBatch)
{
    wasm::doFaasmSmReduce(varPtr, varType, reduceOp, currentBatch);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_sm_critical_local",
                               void,
                               __faasm_sm_critical_local)
{
    wasm::doFaasmSmCriticalLocal();
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_sm_critical_local_end",
                               void,
                               __faasm_sm_critical_local_end)
{
    wasm::doFaasmSmCriticalLocalEnd();
}

// ------------------------------------
// MIGRATION
// ------------------------------------

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_migrate_point",
                               void,
                               __faasm_migrate_point,
                               I32 entrypointFuncPtr,
                               I32 entrypointFuncArg)
{
    SPDLOG_DEBUG(
      "S - faasm_migrate_point {} {}", entrypointFuncPtr, entrypointFuncArg);

    wasm::doMigrationPoint(entrypointFuncPtr,
                           std::to_string(entrypointFuncArg));
}

// ------------------------------------
// LEGACY PYTHON
// ------------------------------------

// 02/12/20 - unfortunately some old Python wasm still needs this
// Emulator API, should not be called from wasm but needs to be present for
// linking
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "setEmulatedMessageFromJson",
                               I32,
                               setEmulatedMessageFromJson,
                               I32 msgPtr)
{
    SPDLOG_DEBUG("S - setEmulatedMessageFromJson - {}", msgPtr);
    throw std::runtime_error(
      "Should not be calling emulator functions from wasm");
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "emulatorGetAsyncResponse",
                               I32,
                               emulatorGetAsyncResponse)
{
    SPDLOG_DEBUG("S - emulatorGetAsyncResponse");
    throw std::runtime_error(
      "Should not be calling emulator functions from wasm");
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "emulatorSetCallStatus",
                               void,
                               emulatorSetCallStatus,
                               I32 success)
{
    SPDLOG_DEBUG("S - emulatorSetCallStatus {}", success);
    throw std::runtime_error(
      "Should not be calling emulator functions from wasm");
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_host_interface_test",
                               void,
                               __faasm_host_interface_test,
                               I32 testNum)
{
    wasm::doHostInterfaceTest(testNum);
}
}
