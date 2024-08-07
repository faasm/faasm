#include "WAVMWasmModule.h"
#include "syscalls.h"

#include <faabric/executor/ExecutorContext.h>
#include <faabric/util/bytes.h>
#include <faabric/util/logging.h>

#include <wasm/chaining.h>

#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Runtime.h>

using namespace WAVM;
using namespace faabric::executor;

namespace wasm {
void chainLink() {}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_await_call",
                               I32,
                               __faasm_await_call,
                               U32 messageId)
{
    SPDLOG_DEBUG("S - await_call - {}", messageId);

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

    SPDLOG_DEBUG(
      "S - await_call_output - {} {} {}", messageId, bufferPtr, bufferLen);

    auto* buffer = &Runtime::memoryRef<char>(
      getExecutingWAVMModule()->defaultMemory, bufferPtr);

    auto result = awaitChainedCallOutput(messageId);

    std::string outputData = result.outputdata();
    strncpy(buffer, outputData.c_str(), outputData.size());

    if (bufferLen < outputData.size()) {
        SPDLOG_WARN("Undersized output buffer: {} for {} output",
                    bufferLen,
                    outputData.size());
    }

    return result.returnvalue();
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
    SPDLOG_DEBUG("S - chain_name - {} ({}) {} {}",
                 funcName,
                 namePtr,
                 inputDataPtr,
                 inputDataLen);

    const std::vector<uint8_t> inputData =
      getBytesFromWasm(inputDataPtr, inputDataLen);

    uint32_t ret = makeChainedCall(funcName, 0, nullptr, inputData);
    return ret;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "__faasm_chain_ptr",
                               U32,
                               __faasm_chain_ptr,
                               I32 wasmFuncPtr,
                               I32 inputDataPtr,
                               I32 inputDataLen)
{
    SPDLOG_DEBUG(
      "S - chain_ptr - {} {} {}", wasmFuncPtr, inputDataPtr, inputDataLen);

    faabric::Message* call = &ExecutorContext::get()->getMsg();
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
    SPDLOG_DEBUG(
      "S - chain_py - {} {} {}", pyFuncName, inputDataPtr, inputDataLen);

    faabric::Message* call = &ExecutorContext::get()->getMsg();
    const std::vector<uint8_t> inputData =
      getBytesFromWasm(inputDataPtr, inputDataLen);

    return makeChainedCall(call->function(), 0, pyFuncName.c_str(), inputData);
}
}
