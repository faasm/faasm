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
    UNIMPLEMENTED_WASM_INTRINSIC_VOID("proc_exit");
}

static NativeSymbol wasiNs[] = {
    REG_WASI_NATIVE_FUNC(args_get, "(**)i"),
    REG_WASI_NATIVE_FUNC(args_sizes_get, "(**)i"),
    REG_WASI_NATIVE_FUNC(proc_exit, "(i)"),
};

uint32_t getFaasmWasiEnvApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = wasiNs;
    return sizeof(wasiNs) / sizeof(NativeSymbol);
}
}
