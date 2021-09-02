#include <faabric/util/logging.h>
#include <wamr/native.h>

#include <stdexcept>
#include <wasm_export.h>

namespace wasm {
uint32_t wasi_clock_time_get(wasm_exec_env_t exec_env,
                             int32_t a,
                             int64_t b,
                             int32_t* c)
{
    SPDLOG_DEBUG("S - clock_time_get");
    throw std::runtime_error("clock_time_get not implemented");
    return 0;
}

uint32_t wasi_poll_oneoff(wasm_exec_env_t exec_env,
                          int32_t* a,
                          int64_t* b,
                          int32_t c,
                          int32_t* d)
{
    SPDLOG_DEBUG("S - poll_oneoff");
    throw std::runtime_error("poll_oneoff not implemented");
    return 0;
}

static NativeSymbol wasiNs[] = {
    REG_WASI_NATIVE_FUNC(clock_time_get, "(iI*)i"),
    REG_WASI_NATIVE_FUNC(poll_oneoff, "(**i*)i"),
};

uint32_t getFaasmWasiTimingApi(NativeSymbol** nativeSymbols)
{
    *nativeSymbols = wasiNs;
    return sizeof(wasiNs) / sizeof(NativeSymbol);
}
}
