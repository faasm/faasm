#include "logging.h"
#include "config.h"
#include "environment.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace  util {
    static std::shared_ptr<spdlog::logger> logger;
    static bool isInitialised = false;

    void initLogging() {
        if(isInitialised) {
            return;
        }

        // Must ensure we have the correct locale set
        util::setLocale();

        // Initialise the logger
        logger = spdlog::stderr_color_mt("console");

        // Work out log level from environment
        SystemConfig &conf = util::getSystemConfig();
        if(conf.logLevel == "debug") {
            spdlog::set_level(spdlog::level::debug);
        }
        else if(conf.logLevel == "trace") {
            spdlog::set_level(spdlog::level::trace);
        }
        else if(conf.logLevel == "off") {
            spdlog::set_level(spdlog::level::off);
        }
        else {
            spdlog::set_level(spdlog::level::info);
        }

        isInitialised = true;
    }

    std::shared_ptr<spdlog::logger> getLogger() {
        if(!isInitialised) {
            initLogging();
        }

        return logger;
    }
}