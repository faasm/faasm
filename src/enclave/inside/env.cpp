#include <enclave/inside/EnclaveWasmModule.h>
#include <enclave/inside/native.h>
#include <enclave/inside/ocalls.h>

namespace sgx {
static int wasi_args_get(wasm_exec_env_t execEnv,
                         uint32_t* argvOffsetsWasm,
                         char* argvBuffWasm)
{
    SPDLOG_DEBUG_SGX("S - wasi_args_get");

    GET_EXECUTING_MODULE_AND_CHECK(execEnv);

    module->writeArgvToWamrMemory(argvOffsetsWasm, argvBuffWasm);

    return 0;
}

static int wasi_args_sizes_get(wasm_exec_env_t execEnv,
                               uint32_t* argcWasm,
                               uint32_t* argvBuffSizeWasm)
{
    SPDLOG_DEBUG_SGX("S - wasi_args_sizes_get");

    GET_EXECUTING_MODULE_AND_CHECK(execEnv);

    *argcWasm = module->getArgc();
    *argvBuffSizeWasm = module->getArgvBufferSize();

    return 0;
}

static void wasi_proc_exit(wasm_exec_env_t execEnv, int returnCode)
{
    auto module = wasm::getExecutingEnclaveWasmModule(execEnv);
    auto* moduleInstance = module->getModuleInstance();
    WASIContext* wasiCtx = wasm_runtime_get_wasi_ctx(moduleInstance);
    wasm_runtime_set_exception(moduleInstance, "wasi proc exit");
    wasiCtx->exit_code = returnCode;
}

static uint32_t wasi_random_get(wasm_exec_env_t execEnv,
                                void* buf,
                                uint32_t bufLen)
{
    SPDLOG_DEBUG_SGX("S - wasi_random_get");
    GET_EXECUTING_MODULE_AND_CHECK(execEnv);
    module->validateNativePointer(buf, bufLen);

    sgx_read_rand(reinterpret_cast<unsigned char*>(buf), bufLen);

    return 0;
}

static NativeSymbol wasiNs[] = {
    REG_WASI_NATIVE_FUNC(args_get, "(**)i"),
    REG_WASI_NATIVE_FUNC(args_sizes_get, "(**)i"),
    REG_WASI_NATIVE_FUNC(proc_exit, "(i)"),
    REG_WASI_NATIVE_FUNC(random_get, "(*~)i"),
};

uint32_t getFaasmWasiEnvApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = wasiNs;
    return sizeof(wasiNs) / sizeof(NativeSymbol);
}
}
