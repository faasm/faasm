#pragma once

#include <chrono>

namespace util {
    typedef std::chrono::steady_clock::time_point TimePoint;

    class Clock {
    public:
        Clock();

        const TimePoint now();

        const long epochNow();

        const long timeDiff(const TimePoint &t1, const TimePoint &t2);

        const long timeDiffNano(const TimePoint &t1, const TimePoint &t2);

        const long timeDiffMicro(const TimePoint &t1, const TimePoint &t2);

        void setFakeNow(const TimePoint &t);

    private:
        bool isFake = false;
        TimePoint fakeNow;

    };

    Clock &getGlobalClock();
}