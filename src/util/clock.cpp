#include "util.h"

namespace util {
    Clock& getGlobalClock() {
        static Clock instance;
        return instance;
    }

    Clock::Clock()  = default;

    const TimePoint Clock::now() {
        if(isFake) {
            return fakeNow;
        }
        else {
            return std::chrono::steady_clock::now();
        }
    }

    const long Clock::timeDiff(const TimePoint &t1, const TimePoint &t2) {
        long age = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t2).count();
        return age;
    }

    void Clock::setFakeNow(const util::TimePoint &t) {
        isFake = true;
        fakeNow = t;
    }
}
