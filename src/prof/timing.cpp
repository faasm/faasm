#include "prof/prof.h"

#include <util/logging.h>

namespace prof {
    util::TimePoint startTimer() {
        util::Clock &clock = util::getGlobalClock();
        return clock.now();
    }

    void logEndTimer(const std::string &label, const util::TimePoint &begin) {
        util::Clock &clock = util::getGlobalClock();
        util::TimePoint end = clock.now();

        long time = clock.timeDiffMicro(end, begin);

        const std::shared_ptr<spdlog::logger> &l = util::getLogger();
        l->trace("{} - {}", label, time);
    }
}