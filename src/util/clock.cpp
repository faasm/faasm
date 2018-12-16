#include "util.h"

namespace util {
    Clock& getGlobalClock() {
        static Clock instance;
        return instance;
    }

    const TimePoint Clock::now() {
        TimePoint begin = std::chrono::steady_clock::now();
        return begin;
    }

    const long Clock::timeDiff(const TimePoint &t1, const TimePoint &t2) {
        long age = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t2).count();
        return age;
    }
}
