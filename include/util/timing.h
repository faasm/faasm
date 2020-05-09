#pragma once

#include <util/clock.h>
#include <string>

#ifdef PROFILE_ALL
#define PROF_START(name) const util::TimePoint name = util::startTimer();
#define PROF_END(name) util::logEndTimer(#name, name);
#else
#define PROF_START(name)
#define PROF_END(name)
#endif

namespace util {
    util::TimePoint startTimer();

    long getTimeDiffMicros(const util::TimePoint &begin);

    double getTimeDiffMillis(const util::TimePoint &begin);

    void logEndTimer(const std::string &label, const util::TimePoint &begin);

    uint64_t timespecToNanos(struct timespec *nativeTimespec);

    void nanosToTimespec(uint64_t nanos, struct timespec *nativeTimespec);
}
