#include "runner/timing.h"

#include <util/logging.h>

namespace runner {
    util::TimePoint startTimer() {
        util::Clock &clock = util::getGlobalClock();
        return clock.now();
    }

    long getTimeDiffMicros(const util::TimePoint &begin) {
        util::Clock &clock = util::getGlobalClock();
        util::TimePoint end = clock.now();

        long micros = clock.timeDiffMicro(end, begin);
        return micros;
    }

    double getTimeDiffMillis(const util::TimePoint &begin) {
        long micros = getTimeDiffMicros(begin);
        double millis = ((double) micros) / 1000;
        return millis;
    }

    void logEndTimer(const std::string &label, const util::TimePoint &begin) {
        double millis = getTimeDiffMillis(begin);
        const std::shared_ptr<spdlog::logger> &l = util::getLogger();
        l->info("TIME = {:.2f}ms ({})", millis, label);
    }
}