#pragma once

#include <string>

namespace util {
    std::string getNodeId();

    std::size_t getNodeIdHash();

    const int DEFAULT_TIMEOUT = 60000;
    const int RESULT_KEY_EXPIRY = 30000;

    class SystemConfig {

    public:
        // System
        std::string hostType;
        std::string globalMessageBus;
        std::string functionStorage;
        std::string fileserverUrl;
        std::string serialisation;
        std::string bucketName;
        std::string queueName;
        std::string cgroupMode;
        std::string netNsMode;
        std::string logLevel;
        std::string awsLogLevel;
        std::string unsafeMode;

        // Redis
        std::string redisStateHost;
        std::string redisQueueHost;
        std::string redisPort;

        // Caching
        std::string zygoteMode;
        std::string irCacheMode;

        // Scheduling
        int maxNodes;
        int threadsPerWorker;
        int noScheduler;
        int prewarm;
        int maxQueueRatio;
        int maxWorkersPerFunction;

        // Worker-related timeouts
        int globalMessageTimeout;
        int unboundTimeout;
        int boundTimeout;

        // State
        int stateStaleThreshold;
        int stateClearThreshold;
        int statePushInterval;
        int fullAsync;
        int fullSync;

        SystemConfig();

        void print();

        void reset();

    private:
        int getSystemConfIntParam(const char *name, const char *defaultValue);

        void initialise();
    };

    SystemConfig &getSystemConfig();
}