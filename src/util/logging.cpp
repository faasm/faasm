#include "util/util.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace  util {
    static std::shared_ptr<spdlog::logger> logger;

    void initLogging() {
        logger = spdlog::stderr_color_mt("console");

        //TODO make log level configurable
        spdlog::set_level(spdlog::level::info);
    }

    std::shared_ptr<spdlog::logger> getLogger() {

        return logger;
    }
}