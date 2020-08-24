#pragma once

#include <faabric/util/clock.h>
#include <string>

#ifdef TRACE_ALL
#define PROF_START(name) const faabric::utilTimePoint name = faabric::utilstartTimer();
#define PROF_END(name) faabric::utillogEndTimer(#name, name);
#else
#define PROF_START(name)
#define PROF_END(name)
#endif

namespace faabric::util {
    faabric::utilTimePoint startTimer();

    long getTimeDiffNanos(const faabric::utilTimePoint &begin);

    long getTimeDiffMicros(const faabric::utilTimePoint &begin);

    double getTimeDiffMillis(const faabric::utilTimePoint &begin);

    void logEndTimer(const std::string &label, const faabric::utilTimePoint &begin);

    uint64_t timespecToNanos(struct timespec *nativeTimespec);

    void nanosToTimespec(uint64_t nanos, struct timespec *nativeTimespec);
}
