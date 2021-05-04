#include "WAVMWasmModule.h"
#include "syscalls.h"

#include <faabric/scheduler/Scheduler.h>
#include <wasm/chaining.h>

#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Runtime.h>
#include <faabric/util/bytes.h>

using namespace WAVM;

namespace wasm {
void chainLink() {}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_await_call",
                               I32,
                               __faasm_await_call,
                               U32 messageId)
{
    faabric::util::getLogger()->debug("S - await_call - {}", messageId);

    return awaitChainedCall(messageId);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_await_call_output",
                               I32,
                               __faasm_await_call_output,
                               U32 messageId,
                               I32 bufferPtr,
                               I32 bufferLen)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();
    logger->debug(
      "S - await_call_output - {} {} {}", messageId, bufferPtr, bufferLen);

    auto buffer = &Runtime::memoryRef<uint8_t>(
      getExecutingWAVMModule()->defaultMemory, bufferPtr);

    return awaitChainedCallOutput(messageId, buffer, bufferLen);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_chain_name",
                               U32,
                               __faasm_chain_name,
                               I32 namePtr,
                               I32 inputDataPtr,
                               I32 inputDataLen)
{
    std::string funcName = getStringFromWasm(namePtr);
    faabric::util::getLogger()->debug(
      "S - chain_name - {} {} {}", funcName, inputDataPtr, inputDataLen);

    const std::vector<uint8_t> inputData =
    getBytesFromWasm(inputDataPtr, inputDataLen);

    return makeChainedCall(funcName, 0, nullptr, inputData);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_chain_ptr",
                               U32,
                               __faasm_chain_ptr,
                               I32 wasmFuncPtr,
                               I32 inputDataPtr,
                               I32 inputDataLen)
{
    faabric::util::getLogger()->debug(
      "S - chain_ptr - {} {} {}", wasmFuncPtr, inputDataPtr, inputDataLen);

    faabric::Message* call = getExecutingCall();
    const std::vector<uint8_t> inputData =
      getBytesFromWasm(inputDataPtr, inputDataLen);

    return makeChainedCall(call->function(), wasmFuncPtr, nullptr, inputData);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_chain_py",
                               U32,
                               __faasm_chain_py,
                               I32 namePtr,
                               I32 inputDataPtr,
                               I32 inputDataLen)
{
    const std::string pyFuncName = getStringFromWasm(namePtr);
    faabric::util::getLogger()->debug(
      "S - chain_py - {} {} {}", pyFuncName, inputDataPtr, inputDataLen);

    faabric::Message* call = getExecutingCall();
    const std::vector<uint8_t> inputData =
      getBytesFromWasm(inputDataPtr, inputDataLen);

    return makeChainedCall(call->function(), 0, pyFuncName.c_str(), inputData);
}
}
