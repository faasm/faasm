#include "config.h"
#include "environment.h"
#include "logging.h"


namespace util {
    SystemConfig &getSystemConfig() {
        static SystemConfig conf;
        return conf;
    }

    SystemConfig::SystemConfig() {
        // TODO - max cannot exceed the underlying number of available namespaces. Good to decouple?
        threadsPerWorker = this->getSystemConfParam("THREADS_PER_WORKER", "50");

        // Scheduling
        prewarm = this->getSystemConfParam("PREWARM", "1");
        maxQueueRatio = this->getSystemConfParam("MAX_QUEUE_RATIO", "3");
        maxWorkersPerFunction = this->getSystemConfParam("MAX_WORKERS_PER_FUNCTION", "10");

        // Worker-related timeouts
        boundTimeout = this->getSystemConfParam("BOUND_TIMEOUT", "30");
        unboundTimeout = this->getSystemConfParam("UNBOUND_TIMEOUT", "5000");

        // State
        stateStaleThreshold = this->getSystemConfParam("STATE_STALE_THRESHOLD", "60000");
        stateClearThreshold = this->getSystemConfParam("STATE_CLEAR_THRESHOLD", "300000");
        statePushInterval = this->getSystemConfParam("STATE_PUSH_INTERVAL", "500");
        fullAsync = this->getSystemConfParam("FULL_ASYNC", "0");
    }

    int SystemConfig::getSystemConfParam(const char *name, const char *defaultValue) {
        int value = stoi(getEnvVar(name, defaultValue));

        return value;
    };

    void SystemConfig::print() {
        const std::shared_ptr<spdlog::logger> &logger = getLogger();

        logger->info("--- Scheduling ---");
        logger->info("THREADS_PER_WORKER         {}", threadsPerWorker);
        logger->info("PREWARM                    {}", prewarm);
        logger->info("MAX_QUEUE_RATIO            {}", maxQueueRatio);
        logger->info("MAX_WORKERS_PER_FUNCTION   {}", maxWorkersPerFunction);

        logger->info("--- Timeouts ---");
        logger->info("BOUND_TIMEOUT              {}", boundTimeout);
        logger->info("UNBOUND_TIMEOUT            {}", unboundTimeout);

        logger->info("--- State ---");
        logger->info("STATE_STALE_THRESHOLD      {}", stateStaleThreshold);
        logger->info("STATE_CLEAR_THRESHOLD      {}", stateClearThreshold);
        logger->info("STATE_PUSH_INTERVAL        {}", statePushInterval);
        logger->info("FULL_ASYNC                 {}", fullAsync);

    }

    std::string getHostName() {
        std::string hostname = util::getEnvVar("HOSTNAME", "");

        if (hostname.empty()) {
            throw std::runtime_error("HOSTNAME for this machine is empty");
        }

        return hostname;
    }
}
