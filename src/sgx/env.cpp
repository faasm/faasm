#include <sgx/native.h>

namespace sgx {
static int args_get_wrapper(wasm_exec_env_t exec_env, int a, int b)
{
    return 0;
}

static int args_sizes_get_wrapper(wasm_exec_env_t exec_env, int a, int b)
{
    return 0;
}

static void proc_exit_wrapper(wasm_exec_env_t exec_env, int returnCode) {}

static NativeSymbol wasiNs[] = {
    REG_WASI_NATIVE_FUNC(args_get, "(ii)i"),
    REG_WASI_NATIVE_FUNC(args_sizes_get, "(ii)i"),
    REG_WASI_NATIVE_FUNC(proc_exit, "(i)"),
};

uint32_t getFaasmWasiEnvApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = wasiNs;
    return sizeof(wasiNs) / sizeof(NativeSymbol);
}
}
