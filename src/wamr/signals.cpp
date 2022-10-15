#include <faabric/util/logging.h>
#include <wamr/native.h>
#include <wasm/WasmEnvironment.h>

#include <wasm_export.h>

namespace wasm {
static uint32_t signal_wrapper(wasm_exec_env_t exec_env, uint32_t a, uint32_t b)
{
    SPDLOG_DEBUG("S - signal");

    return 0;
}

static NativeSymbol ns[] = {
    REG_NATIVE_FUNC(signal, "(ii)i"),
};

uint32_t getFaasmSignalApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = ns;
    return sizeof(ns) / sizeof(NativeSymbol);
}
}
