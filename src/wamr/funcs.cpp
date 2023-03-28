#include <faabric/proto/faabric.pb.h>
#include <faabric/scheduler/ExecutorContext.h>
#include <faabric/util/bytes.h>
#include <faabric/util/logging.h>
#include <faabric/util/macros.h>
#include <wamr/WAMRWasmModule.h>
#include <wamr/native.h>
#include <wasm/WasmExecutionContext.h>
#include <wasm/WasmModule.h>
#include <wasm/chaining.h>

#include <wasm_export.h>

using namespace faabric::scheduler;

namespace wasm {

static std::shared_ptr<faabric::state::StateKeyValue> getStateKV(
  int32_t* keyPtr,
  size_t size)
{
    WAMRWasmModule* module = getExecutingWAMRModule();
    module->validateNativePointer(keyPtr, sizeof(int32_t));

    const faabric::Message* call = &ExecutorContext::get()->getMsg();
    char* key = reinterpret_cast<char*>(keyPtr); // second

    faabric::state::State& s = faabric::state::getGlobalState();
    auto kv = s.getKV(call->user(), key, size);

    return kv;
}

/**
 * Await a chained function's completion
 */
static int32_t __faasm_await_call_wrapper(wasm_exec_env_t exec_env,
                                          int32_t callId)
{
    SPDLOG_DEBUG("S - faasm_await_call {}", callId);

    int32_t result = wasm::awaitChainedCall((uint32_t)callId);
    return result;
}

/**
 * Chain a function by function pointer
 */
static int32_t __faasm_chain_ptr_wrapper(wasm_exec_env_t exec_env,
                                         int32_t wasmFuncPtr,
                                         char* inBuff,
                                         int32_t inLen)
{
    SPDLOG_DEBUG("S - faasm_chain_ptr {} {} {}", wasmFuncPtr, inBuff, inLen);

    faabric::Message& call = ExecutorContext::get()->getMsg();
    std::vector<uint8_t> inputData(BYTES(inBuff), BYTES(inBuff) + inLen);
    return makeChainedCall(call.function(), wasmFuncPtr, nullptr, inputData);
}

static void __faasm_pull_state_wrapper(wasm_exec_env_t execEnv,
                                       int32_t* keyPtr,
                                       int32_t stateLen)
{
    auto kv = getStateKV(keyPtr, stateLen);
    SPDLOG_DEBUG("S - pull_state - {} {}", kv->key, stateLen);

    kv->pull();
}

static void __faasm_push_state_wrapper(wasm_exec_env_t execEnv, int32_t* keyPtr)
{
    auto kv = getStateKV(keyPtr, 0);
    SPDLOG_DEBUG("S - push_state - {}", kv->key);
    kv->pushFull();
}

/**
 * Read the function input
 */
static int32_t __faasm_read_input_wrapper(wasm_exec_env_t exec_env,
                                          char* inBuff,
                                          int32_t inLen)
{
    SPDLOG_DEBUG("S - faasm_read_input {} {}", inBuff, inLen);

    faabric::Message& call = ExecutorContext::get()->getMsg();
    std::vector<uint8_t> inputBytes =
      faabric::util::stringToBytes(call.inputdata());

    // If nothing, return nothing
    if (inputBytes.empty()) {
        return 0;
    }

    // Write to the wasm buffer
    int inputSize = faabric::util::safeCopyToBuffer(
      inputBytes, reinterpret_cast<uint8_t*>(inBuff), inLen);
    return inputSize;
}

/**
 * Set the function output
 */
static void __faasm_write_output_wrapper(wasm_exec_env_t exec_env,
                                         char* outBuff,
                                         int32_t outLen)
{
    SPDLOG_DEBUG("S - faasm_write_output {} {}", outBuff, outLen);

    faabric::Message& call = ExecutorContext::get()->getMsg();
    call.set_outputdata(outBuff, outLen);
}

static NativeSymbol ns[] = {
    REG_NATIVE_FUNC(__faasm_await_call, "(i)i"),
    REG_NATIVE_FUNC(__faasm_chain_ptr, "(i$i)i"),
    REG_NATIVE_FUNC(__faasm_pull_state, "(*i)"),
    REG_NATIVE_FUNC(__faasm_push_state, "(*)"),
    REG_NATIVE_FUNC(__faasm_read_input, "($i)i"),
    REG_NATIVE_FUNC(__faasm_write_output, "($i)"),
};

uint32_t getFaasmFunctionsApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = ns;
    return sizeof(ns) / sizeof(NativeSymbol);
}
}
