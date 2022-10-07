#include <faabric/util/logging.h>
#include <faabric/util/timing.h>
#include <wamr/native.h>
#include <wamr/types.h>

#include <stdexcept>
#include <sys/time.h>
#include <wasm_export.h>

namespace wasm {
uint32_t wasi_clock_time_get(wasm_exec_env_t exec_env,
                             int32_t clockId,
                             int64_t precision,
                             int32_t* result)
{
    SPDLOG_DEBUG("S - clock_time_get");

    timespec ts{};

    // This switch statement is duplicated in src/wavm/timing.cpp the reason
    // being that, even though the constants should be the same, they are
    // defined in runtime-specific headers. Instead of mixing them up, we
    // keep the code duplicated
    int linuxClockId;
    switch (clockId) {
        case __WASI_CLOCK_REALTIME:
            linuxClockId = CLOCK_REALTIME;
            break;
        case __WASI_CLOCK_MONOTONIC:
            linuxClockId = CLOCK_MONOTONIC;
            break;
        case __WASI_CLOCK_PROCESS_CPUTIME_ID:
            linuxClockId = CLOCK_PROCESS_CPUTIME_ID;
            break;
        case __WASI_CLOCK_THREAD_CPUTIME_ID:
            linuxClockId = CLOCK_THREAD_CPUTIME_ID;
            break;
        default:
            SPDLOG_ERROR("Unknown clock ID: {}", clockId);
            throw std::runtime_error("Unknown clock ID");
    }

    int retVal = clock_gettime(linuxClockId, &ts);
    if (retVal < 0) {
        if (EINVAL) {
            return __WASI_EINVAL;
        } else {
            SPDLOG_ERROR("Unexpected clock error: {}", retVal);
            throw std::runtime_error("Unexpected clock error");
        }
    }

    *result = faabric::util::timespecToNanos(&ts);

    return __WASI_ESUCCESS;
}

uint32_t wasi_poll_oneoff(wasm_exec_env_t exec_env,
                          int32_t* a,
                          int64_t* b,
                          int32_t c,
                          int32_t* d)
{
    SPDLOG_DEBUG("S - poll_oneoff");
    throw std::runtime_error("poll_oneoff not implemented");
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
