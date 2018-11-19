#include "prof/prof.h"

namespace prof {
    std::chrono::steady_clock::time_point startTimer() {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        return begin;
    }

    void logEndTimer(const std::string &label, const std::chrono::steady_clock::time_point &begin) {
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

        long time = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

        const std::shared_ptr<spdlog::logger> &l = util::getLogger();
        l->trace("{} - {}", label, time);
    }
}