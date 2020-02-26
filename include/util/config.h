#pragma once

#include <string>

#define NODE_ID_LEN 20

#define DEFAULT_TIMEOUT 60000
#define RESULT_KEY_EXPIRY 30000

namespace util {
    std::string getNodeId();

    std::size_t getNodeIdHash();

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
        std::string pythonPreload;
        std::string captureStdout;

        // Redis
        std::string redisStateHost;
        std::string redisQueueHost;
        std::string redisPort;

        // Caching
        std::string irCacheMode;

        // Scheduling
        int maxNodes;
        int threadsPerWorker;
        int noScheduler;
        int maxInFlightRatio;
        int maxWorkersPerFunction;

        // Worker-related timeouts
        int globalMessageTimeout;
        int unboundTimeout;
        int boundTimeout;
        int chainedCallTimeout;

        // Filesystem storage
        std::string functionDir;
        std::string objectFileDir;
        std::string runtimeFilesDir;
        std::string sharedFilesDir;
        std::string sharedFilesStorageDir;

        // IBM
        std::string ibmApiKey;

        // MPI
        int mpiWorldSize;

        SystemConfig();

        void print();

        void reset();

    private:
        int getSystemConfIntParam(const char *name, const char *defaultValue);

        void initialise();
    };

    SystemConfig &getSystemConfig();
}