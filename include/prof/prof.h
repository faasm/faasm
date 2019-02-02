#pragma once

#include <util/clock.h>
#include <string>

namespace prof {
    // Timing
    util::TimePoint startTimer();

    void logEndTimer(const std::string &label, const util::TimePoint &begin);
};
