#pragma once

#include <chrono>
#include <util/util.h>

namespace prof {
    // Timing
    std::chrono::steady_clock::time_point startTimer();

    void logEndTimer(const std::string &label, const std::chrono::steady_clock::time_point &begin);
};
