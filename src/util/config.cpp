#include "util.h"

namespace util {
    SystemConfig::SystemConfig() {
        prewarmTarget = this->getSystemConfParam("PREWARM_TARGET", "20");
        maxQueueRatio = this->getSystemConfParam("MAX_QUEUE_RATIO", "4");
        maxWorkersPerFunction = this->getSystemConfParam("MAX_WORKERS_PER_FUNCTION", "10");
        boundTimeout = this->getSystemConfParam("BOUND_TIMEOUT", "30");
        unboundTimeout = this->getSystemConfParam("UNBOUND_TIMEOUT", "240");
        stateStaleThreshold = this->getSystemConfParam("STATE_STALE_THRESHOLD", "3000");
        stateClearThreshold = this->getSystemConfParam("STATE_CLEAR_THRESHOLD", "30000");
        statePushInterval = this->getSystemConfParam("STATE_PUSH_INTERVAL", "10000");
    }

    int SystemConfig::getSystemConfParam(const char *name, const char *defaultValue) {
        int value = stoi(getEnvVar(name, defaultValue));

        return value;
    };

    void SystemConfig::print() {
        const std::shared_ptr<spdlog::logger> &logger = getLogger();

        logger->info("PREWARM_TARGET             {}", prewarmTarget);
        logger->info("MAX_QUEUE_RATIO            {}", maxQueueRatio);
        logger->info("MAX_WORKERS_PER_FUNCTION   {}", maxWorkersPerFunction);
        logger->info("BOUND_TIMEOUT              {}", boundTimeout);
        logger->info("UNBOUND_TIMEOUT            {}", unboundTimeout);
        logger->info("STATE_STALE_THRESHOLD      {}", stateStaleThreshold);
        logger->info("STATE_CLEAR_THRESHOLD      {}", stateClearThreshold);
        logger->info("STATE_PUSH_INTERVAL        {}", statePushInterval);
    }
}
