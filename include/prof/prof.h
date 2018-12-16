#pragma once

#include <util/util.h>

namespace prof {
    // Timing
    util::TimePoint startTimer();

    void logEndTimer(const std::string &label, const util::TimePoint &begin);
};
