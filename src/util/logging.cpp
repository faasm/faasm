#include "util/util.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace  util {
    static std::shared_ptr<spdlog::logger> logger;

    void initLogging() {
        logger = spdlog::stderr_color_mt("console");

        // Work out log level from environment
        std::string logLevelEnv = util::getEnvVar("LOG_LEVEL", "info");
        if(logLevelEnv == "debug") {
            spdlog::set_level(spdlog::level::debug);
        }
        else if(logLevelEnv == "trace") {
            spdlog::set_level(spdlog::level::trace);
        }
        else {
            spdlog::set_level(spdlog::level::info);
        }
    }

    std::shared_ptr<spdlog::logger> getLogger() {
        return logger;
    }
}