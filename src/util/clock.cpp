#include "clock.h"

namespace util {
    Clock &getGlobalClock() {
        static Clock instance;
        return instance;
    }

    Clock::Clock() = default;

    const TimePoint Clock::now() {
        return std::chrono::steady_clock::now();
    }

    const long Clock::epochMillis() {
        long millis = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
        ).count();

        return millis;
    }

    const long Clock::timeDiff(const TimePoint &t1, const TimePoint &t2) {
        long age = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t2).count();
        return age;
    }

    const long Clock::timeDiffMicro(const TimePoint &t1, const TimePoint &t2) {
        long age = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t2).count();
        return age;
    }

    const long Clock::timeDiffNano(const TimePoint &t1, const TimePoint &t2) {
        long age = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t2).count();
        return age;
    }
}
