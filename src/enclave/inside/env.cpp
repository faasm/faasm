#include <enclave/inside/EnclaveWasmModule.h>
#include <enclave/inside/native.h>
#include <enclave/inside/ocalls.h>

#include <string>

namespace sgx {
static int wasi_args_get(wasm_exec_env_t exec_env,
                         uint32_t* argvOffsetsWasm,
                         char* argvBuffWasm)
{
    ocallLogDebug("S - wasi_args_get");

    GET_EXECUTING_MODULE_AND_CHECK(exec_env);

    module->writeArgvToWamrMemory(argvOffsetsWasm, argvBuffWasm);

    return 0;
}

static int wasi_args_sizes_get(wasm_exec_env_t exec_env,
                               uint32_t* argcWasm,
                               uint32_t* argvBuffSizeWasm)
{
    ocallLogDebug("S - wasi_args_sizes_get");

    GET_EXECUTING_MODULE_AND_CHECK(exec_env);

    *argcWasm = module->getArgc();
    *argvBuffSizeWasm = module->getArgvBufferSize();

    return 0;
}

static void wasi_proc_exit(wasm_exec_env_t exec_env, int returnCode) {}

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
