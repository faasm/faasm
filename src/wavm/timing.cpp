#include "WAVMWasmModule.h"
#include "syscalls.h"

#include <sys/time.h>

#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/WASI/WASIABI.h>
#include <faabric/util/timing.h>

using namespace WAVM;

namespace wasm {
    //TODO - make timing functions more secure
    I32 s__clock_gettime(I32 clockId, I32 timespecPtr) {
        faabric::utilgetLogger()->debug("S - clock_gettime - {} {}", clockId, timespecPtr);

        timespec ts{};
        int retVal = clock_gettime(clockId, &ts);
        if (retVal == -1) {
            int systemErrno = errno;
            faabric::utilgetLogger()->error("Clock type not supported - {} ({})\n", systemErrno, strerror(systemErrno));
            return -systemErrno;
        }

        auto result = &Runtime::memoryRef<wasm_timespec>(getExecutingWAVMModule()->defaultMemory, (Uptr) timespecPtr);
        result->tv_sec = I64(ts.tv_sec);
        result->tv_nsec = I32(ts.tv_nsec);

        return 0;
    }

    /**
     * As specified in the gettimeofday man page, use of the timezone struct is obsolete and hence not supported here
     */
    I32 s__gettimeofday(int tvPtr, int tzPtr) {
        faabric::utilgetLogger()->debug("S - gettimeofday - {} {}", tvPtr, tzPtr);

        timeval tv{};
        gettimeofday(&tv, nullptr);

        auto result = &Runtime::memoryRef<wasm_timeval>(getExecutingWAVMModule()->defaultMemory, (Uptr) tvPtr);
        result->tv_sec = I32(tv.tv_sec);
        result->tv_usec = I32(tv.tv_usec);

        return 0;
    }

    /**
     * Allow sleep for now
     */
    I32 s__nanosleep(I32 reqPtr, I32 remPtr) {
        faabric::utilgetLogger()->debug("S - nanosleep - {} {}", reqPtr, remPtr);

        auto request = &Runtime::memoryRef<wasm_timespec>(getExecutingWAVMModule()->defaultMemory, (Uptr) reqPtr);

        // TODO - is this ok? Should we allow?
        // Round up
        sleep(request->tv_sec + 1);

        return 0;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "poll_oneoff", I32, wasi_poll_oneoff, I32 subscriptionsPtr, I32 eventsPtr,
                                   I32 nSubs,
                                   I32 resNEvents) {
        faabric::utilgetLogger()->debug("S - poll_oneoff - {} {} {} {}", subscriptionsPtr, eventsPtr, nSubs, resNEvents);
        WAVMWasmModule *module = getExecutingWAVMModule();

        auto inEvents = Runtime::memoryArrayPtr<__wasi_subscription_t>(module->defaultMemory, subscriptionsPtr, nSubs);
        auto outEvents = Runtime::memoryArrayPtr<__wasi_event_t>(module->defaultMemory, eventsPtr, nSubs);

        for (int i = 0; i < nSubs; i++) {
            __wasi_subscription_t *thisSub = &inEvents[i];

            if (thisSub->type == __WASI_EVENTTYPE_CLOCK) {
                // This is a timing event like a sleep
                uint64_t timeoutNanos = thisSub->u.clock.timeout;
                int clockType = 0;
                if (thisSub->u.clock.clock_id == __WASI_CLOCK_MONOTONIC) {
                    clockType = CLOCK_MONOTONIC;
                } else if (thisSub->u.clock.clock_id == __WASI_CLOCK_REALTIME) {
                    clockType = CLOCK_REALTIME;
                } else {
                    throw std::runtime_error("Unimplemented clock type");
                }

                // Do the sleep
                timespec t{};
                faabric::utilnanosToTimespec(timeoutNanos, &t);
                clock_nanosleep(clockType, 0, &t, nullptr);
            } else {
                throw std::runtime_error("Unimplemented event type");
            }

            // Say that the event has occurred
            __wasi_event_t *thisEvent = &outEvents[i];
            thisEvent->type = thisSub->type;
            thisEvent->error = __WASI_ESUCCESS;
        }

        // Write the result
        Runtime::memoryRef<U32>(module->defaultMemory, resNEvents) = (U32) nSubs;

        return __WASI_ESUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "utime", I32, s__utime, I32 a, I32 b) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "clock_time_get", I32, wasi_clock_time_get, I32 clockId, I64 precision,
                                   I32 resultPtr) {
        faabric::utilgetLogger()->debug("S - clock_time_get - {} {} {}", clockId, precision, resultPtr);

        timespec ts{};

        int linuxClockId;
        if (clockId == __WASI_CLOCK_REALTIME) {
            linuxClockId = CLOCK_REALTIME;
        } else if (clockId == __WASI_CLOCK_MONOTONIC) {
            linuxClockId = CLOCK_MONOTONIC;
        } else if (clockId == __WASI_CLOCK_PROCESS_CPUTIME_ID) {
            linuxClockId = CLOCK_PROCESS_CPUTIME_ID;
        } else if (clockId == __WASI_CLOCK_THREAD_CPUTIME_ID) {
            linuxClockId = CLOCK_THREAD_CPUTIME_ID;
        } else {
            throw std::runtime_error("Unknown clock ID");
        }

        int retVal = clock_gettime(linuxClockId, &ts);
        if (retVal < 0) {
            if (EINVAL) {
                return __WASI_EINVAL;
            } else {
                throw std::runtime_error("Unexpected clock error");
            }
        }

        uint64_t result = faabric::utiltimespecToNanos(&ts);
        Runtime::memoryRef<uint64_t>(getExecutingWAVMModule()->defaultMemory, resultPtr) = result;

        return __WASI_ESUCCESS;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(wasi, "clock_res_get", I32, wasi_clock_res_get, I32 a, I32 b) {
        throwException(Runtime::ExceptionTypes::calledUnimplementedIntrinsic);
    }

    void timingLink() {

    }
}