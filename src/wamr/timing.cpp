#include <faabric/util/logging.h>
#include <faabric/util/timing.h>
#include <wamr/WAMRWasmModule.h>
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
        }

        SPDLOG_ERROR("Unexpected clock error: {}", retVal);
        throw std::runtime_error("Unexpected clock error");
    }

    *result = faabric::util::timespecToNanos(&ts);

    return __WASI_ESUCCESS;
}

uint32_t wasi_poll_oneoff(wasm_exec_env_t exec_env,
                          int32_t* subscriptionsPtr,
                          int64_t* eventsPtr,
                          int32_t nSubs,
                          int32_t* resNEvents)
{
    SPDLOG_DEBUG("S - poll_oneoff");

    WAMRWasmModule* module = getExecutingWAMRModule();

    module->validateNativePointer(subscriptionsPtr,
                                  nSubs * sizeof(__wasi_subscription_t));
    auto* inEvents = reinterpret_cast<__wasi_subscription_t*>(subscriptionsPtr);

    module->validateNativePointer(eventsPtr, nSubs * sizeof(__wasi_event_t));
    auto* outEvents = reinterpret_cast<__wasi_event_t*>(eventsPtr);

    // Note: the WAVM implementation is out of date and does not follow the
    // WASI spec anymore!
    for (int i = 0; i < nSubs; i++) {
        const __wasi_subscription_t* thisSub = &inEvents[i];

        if (thisSub->u.type == __WASI_EVENTTYPE_CLOCK) {
            // This is a timing event like a sleep
            uint64_t timeoutNanos = thisSub->u.u.clock.timeout;
            int clockType = 0;
            if (thisSub->u.u.clock.clock_id == __WASI_CLOCK_MONOTONIC) {
                clockType = CLOCK_MONOTONIC;
            } else if (thisSub->u.u.clock.clock_id == __WASI_CLOCK_REALTIME) {
                clockType = CLOCK_REALTIME;
            } else {
                throw std::runtime_error("Unimplemented clock type");
            }

            // Do the sleep
            timespec t{};
            faabric::util::nanosToTimespec(timeoutNanos, &t);
            clock_nanosleep(clockType, 0, &t, nullptr);
        } else {
            throw std::runtime_error("Unimplemented event type");
        }

        // Say that the event has occurred
        __wasi_event_t* thisEvent = &outEvents[i];
        thisEvent->type = thisSub->u.type;
        thisEvent->error = __WASI_ESUCCESS;
    }

    *resNEvents = nSubs;

    return __WASI_ESUCCESS;
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
