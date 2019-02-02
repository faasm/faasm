#pragma once

#include <spdlog/spdlog.h>

namespace util {
    void initLogging();

    std::shared_ptr<spdlog::logger> getLogger();
}