#include <faabric/executor/ExecutorContext.h>
#include <faabric/proto/faabric.pb.h>
#include <faabric/util/bytes.h>
#include <faabric/util/logging.h>
#include <faabric/util/macros.h>
#include <wamr/WAMRWasmModule.h>
#include <wamr/native.h>
#include <wasm/WasmExecutionContext.h>
#include <wasm/WasmModule.h>
#include <wasm/chaining.h>
#include <wasm/faasm.h>
#include <wasm/host_interface_test.h>
#include <wasm/migration.h>

#include <wasm_export.h>

using namespace faabric::executor;

namespace wasm {

static std::shared_ptr<faabric::state::StateKeyValue> getStateKV(
  int32_t* keyPtr,
  size_t size = 0)
{
    WAMRWasmModule* module = getExecutingWAMRModule();
    module->validateNativePointer(keyPtr, sizeof(int32_t));

    const faabric::Message* call = &ExecutorContext::get()->getMsg();
    char* key = reinterpret_cast<char*>(keyPtr); // second

    faabric::state::State& s = faabric::state::getGlobalState();
    std::shared_ptr<faabric::state::StateKeyValue> kv;
    if (size > 0) {
        kv = s.getKV(call->user(), key, size);
    } else {
        kv = s.getKV(call->user(), key);
    }

    return kv;
}

static void __faasm_append_state_wrapper(wasm_exec_env_t execEnv,
                                         int32_t* keyPtr,
                                         uint8_t* dataPtr,
                                         int32_t dataLen)
{
    auto* module = getExecutingWAMRModule();
    module->validateNativePointer(dataPtr, dataLen);

    SPDLOG_DEBUG("S - faasm_append_state {}", (char*)keyPtr);

    auto kv = getStateKV(keyPtr);
    kv->append(dataPtr, dataLen);
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
 * Await a chained function's completion, and return its output.
 *
 * Note that, in reality, bufferPtr is a char** and we need to malloc
 * the WASM memory from the host.
 */
static int32_t __faasm_await_call_output_wrapper(wasm_exec_env_t execEnv,
                                                 int32_t callId,
                                                 int32_t* bufferPtr,
                                                 int32_t* bufferLen)
{
    SPDLOG_DEBUG("S - faasm_await_call_output {}", callId);
    auto* module = getExecutingWAMRModule();

    faabric::Message result;
    try {
        result = awaitChainedCallOutput(callId);
    } catch (std::exception& exc) {
        module->doThrowException(exc);
    }

    std::string outputData = result.outputdata();

    // Copy the result into heap-allocated WASM memory
    void* nativePtr = nullptr;
    auto wasmOffset = module->wasmModuleMalloc(outputData.size(), &nativePtr);
    if (wasmOffset == 0 || nativePtr == nullptr) {
        SPDLOG_ERROR("Error allocating memory in WASM module");
        auto exc = std::runtime_error("Error allocating memory in module!");
        module->doThrowException(exc);
    }
    std::memcpy(nativePtr, outputData.c_str(), outputData.size());

    // Populate the provided pointers
    *bufferPtr = wasmOffset;
    *bufferLen = outputData.size();

    return result.returnvalue();
}

/**
 * Chain a function by name
 */
static int32_t __faasm_chain_name_wrapper(wasm_exec_env_t execEnv,
                                          const char* name,
                                          const uint8_t* input,
                                          uint32_t inputSize)
{
    std::vector<uint8_t> _input(input, input + inputSize);
    SPDLOG_DEBUG("S - chain_name - {}", std::string(name));
    return wasm::makeChainedCall(std::string(name), 0, nullptr, _input);
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

/*
 * Single entry-point for testing the host interface behaviour
 */
static void __faasm_host_interface_test_wrapper(wasm_exec_env_t execEnv,
                                                int32_t testNum)
{
    wasm::doHostInterfaceTest(testNum);
}

static void __faasm_migrate_point_wrapper(wasm_exec_env_t execEnv,
                                          int32_t wasmFuncPtr,
                                          int32_t funcArg)
{
    SPDLOG_DEBUG("S - faasm_migrate_point {} {}", wasmFuncPtr, funcArg);

    wasm::doMigrationPoint(wasmFuncPtr, std::to_string(funcArg));
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

static void __faasm_read_appended_state_wrapper(wasm_exec_env_t execEnv,
                                                int32_t* keyPtr,
                                                uint8_t* bufferPtr,
                                                int32_t bufferLen,
                                                int32_t numElems)
{
    auto* module = getExecutingWAMRModule();
    module->validateNativePointer(bufferPtr, bufferLen);

    SPDLOG_DEBUG("S - faasm_read_appended_state {}", (char*)keyPtr);

    auto kv = getStateKV(keyPtr, bufferLen);
    kv->getAppended(bufferPtr, bufferLen, numElems);
}

/**
 * Read the function input
 */
static int32_t __faasm_read_input_wrapper(wasm_exec_env_t exec_env,
                                          char* inBuff,
                                          int32_t inLen)
{
    return wasm::doFaasmReadInput(inBuff, inLen);
}

static void __faasm_sm_critical_local_wrapper(wasm_exec_env_t execEnv)
{
    wasm::doFaasmSmCriticalLocal();
}

static void __faasm_sm_critical_local_end_wrapper(wasm_exec_env_t execEnv)
{
    wasm::doFaasmSmCriticalLocalEnd();
}

static void __faasm_sm_reduce_wrapper(wasm_exec_env_t execEnv,
                                      int32_t varPtr,
                                      int32_t varType,
                                      int32_t reduceOp,
                                      int32_t currentBatch)
{
    wasm::doFaasmSmReduce(varPtr, varType, reduceOp, currentBatch);
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

static NativeSymbol faasmNs[] = {
    REG_NATIVE_FUNC(__faasm_append_state, "(**i)"),
    REG_NATIVE_FUNC(__faasm_await_call, "(i)i"),
    REG_NATIVE_FUNC(__faasm_await_call_output, "(i**)i"),
    REG_NATIVE_FUNC(__faasm_chain_name, "($$i)i"),
    REG_NATIVE_FUNC(__faasm_chain_ptr, "(i$i)i"),
    REG_NATIVE_FUNC(__faasm_host_interface_test, "(i)"),
    REG_NATIVE_FUNC(__faasm_migrate_point, "(ii)"),
    REG_NATIVE_FUNC(__faasm_pull_state, "(*i)"),
    REG_NATIVE_FUNC(__faasm_push_state, "(*)"),
    REG_NATIVE_FUNC(__faasm_read_appended_state, "(**ii)"),
    REG_NATIVE_FUNC(__faasm_read_input, "($i)i"),
    REG_NATIVE_FUNC(__faasm_sm_critical_local, "()"),
    REG_NATIVE_FUNC(__faasm_sm_critical_local_end, "()"),
    REG_NATIVE_FUNC(__faasm_sm_reduce, "(iiii)"),
    REG_NATIVE_FUNC(__faasm_write_output, "($i)"),
};

uint32_t getFaasmFunctionsApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = faasmNs;
    return sizeof(faasmNs) / sizeof(NativeSymbol);
}
}
