#include "timing.h"

#include <faabric/util/logging.h>

namespace faabric::util {
    faabric::utilTimePoint startTimer() {
        faabric::utilClock &clock = faabric::utilgetGlobalClock();
        return clock.now();
    }

    long getTimeDiffNanos(const faabric::utilTimePoint &begin) {
        faabric::utilClock &clock = faabric::utilgetGlobalClock();
        return clock.timeDiffNano(clock.now(), begin);
    }

    long getTimeDiffMicros(const faabric::utilTimePoint &begin) {
        faabric::utilClock &clock = faabric::utilgetGlobalClock();
        faabric::utilTimePoint end = clock.now();

        long micros = clock.timeDiffMicro(end, begin);
        return micros;
    }

    double getTimeDiffMillis(const faabric::utilTimePoint &begin) {
        long micros = getTimeDiffMicros(begin);
        double millis = ((double) micros) / 1000;
        return millis;
    }

    void logEndTimer(const std::string &label, const faabric::utilTimePoint &begin) {
        double millis = getTimeDiffMillis(begin);
        const std::shared_ptr<spdlog::logger> &l = faabric::utilgetLogger();
        l->trace("TIME = {:.2f}ms ({})", millis, label);
    }

    uint64_t timespecToNanos(struct timespec *nativeTimespec) {
        uint64_t nanos = nativeTimespec->tv_sec * 1000000000;
        nanos += nativeTimespec->tv_nsec;

        return nanos;
    }

    void nanosToTimespec(uint64_t nanos, struct timespec *nativeTimespec) {
        nativeTimespec->tv_sec = nanos / 1000000000;
        nativeTimespec->tv_nsec = nanos % 1000000000;
    }
}