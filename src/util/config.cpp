#include "util.h"

namespace util {
    SystemConfig::SystemConfig() {
        prewarm_target = this->getSystemConfParam("PREWARM_TARGET", "20");
        max_queue_ratio = this->getSystemConfParam("MAX_QUEUE_RATIO", "4");
        max_workers_per_function = this->getSystemConfParam("MAX_WORKERS_PER_FUNCTION", "10");
        bound_timeout = this->getSystemConfParam("BOUND_TIMEOUT", "30");
        unbound_timeout = this->getSystemConfParam("UNBOUND_TIMEOUT", "240");
    }

    int SystemConfig::getSystemConfParam(const char *name, const char *defaultValue) {
        int value = stoi(getEnvVar(name, defaultValue));

        return value;
    };

    void SystemConfig::print() {
        const std::shared_ptr<spdlog::logger> &logger = getLogger();

        logger->info("PREWARM_TARGET             {}", prewarm_target);
        logger->info("MAX_QUEUE_RATIO            {}", max_queue_ratio);
        logger->info("MAX_WORKERS_PER_FUNCTION   {}", max_workers_per_function);
        logger->info("BOUND_TIMEOUT              {}", bound_timeout);
        logger->info("UNBOUND_TIMEOUT            {}", unbound_timeout);
    }
}
