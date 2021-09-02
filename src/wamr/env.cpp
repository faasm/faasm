#include <faabric/util/logging.h>
#include <wamr/native.h>

#include <stdexcept>
#include <wasm_export.h>

namespace wasm {
uint32_t wasi_environ_get(wasm_exec_env_t exec_env, int32_t* a, int32_t* b)
{
    SPDLOG_DEBUG("S - environ_get");
    throw std::runtime_error("environ_get not implemented");
    return 0;
}

uint32_t wasi_environ_sizes_get(wasm_exec_env_t exec_env,
                                int32_t* a,
                                int32_t* b)
{
    SPDLOG_DEBUG("S - environ_sizes_get");
    throw std::runtime_error("environ_sizes_get not implemented");
    return 0;
}

uint32_t wasi_proc_exit(wasm_exec_env_t exec_env)
{
    SPDLOG_DEBUG("S - proc_exit");
    throw std::runtime_error("proc_exit not implemented");
    return 0;
}

static NativeSymbol wasiNs[] = {
    REG_WASI_NATIVE_FUNC(environ_get, "(**)i"),
    REG_WASI_NATIVE_FUNC(environ_sizes_get, "(**)i"),
    REG_WASI_NATIVE_FUNC(proc_exit, "(i)"),
};

uint32_t getFaasmWasiEnvApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = wasiNs;
    return sizeof(wasiNs) / sizeof(NativeSymbol);
}
}
