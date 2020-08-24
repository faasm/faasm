#pragma once

#include <spdlog/spdlog.h>

namespace faabric::util {
    void initLogging();

    std::shared_ptr<spdlog::logger> getLogger();
}