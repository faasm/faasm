#pragma once

#include <string>

namespace util {
    std::string getHostName();

    const int DEFAULT_TIMEOUT = 60;
    const int RESULT_KEY_EXPIRY = 30;

    class SystemConfig {

    public:
        // Scheduling
        int threadsPerWorker;
        int prewarmTarget;
        int maxQueueRatio;
        int maxWorkersPerFunction;

        // Worker-related timeouts
        int unboundTimeout;
        int boundTimeout;

        // State
        int stateStaleThreshold;
        int stateClearThreshold;
        int statePushInterval;
        int fullAsync;

        SystemConfig();

        void print();

    private:
        int getSystemConfParam(const char *name, const char *defaultValue);
    };

    SystemConfig &getSystemConfig();
}