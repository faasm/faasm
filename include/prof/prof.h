#pragma once

#include <util/clock.h>
#include <string>

namespace prof {
    // Timing
    util::TimePoint startTimer();

    long getTimeDiffMicros(const util::TimePoint &begin);

    double getTimeDiffMillis(const util::TimePoint &begin);

    void logEndTimer(const std::string &label, const util::TimePoint &begin);
};
