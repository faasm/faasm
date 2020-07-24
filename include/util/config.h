#pragma once

#include <string>

#define MPI_HOST_STATE_LEN 20

#define DEFAULT_TIMEOUT 60000
#define RESULT_KEY_EXPIRY 30000

namespace util {
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
        std::string pythonPreload;
        std::string captureStdout;
        std::string stateMode;
        std::string wasmVm;
        std::string execGraphMode;

        // Redis
        std::string redisStateHost;
        std::string redisQueueHost;
        std::string redisPort;

        // Caching
        std::string irCacheMode;

        // Scheduling
        int maxFaaslets;
        int noScheduler;
        int maxInFlightRatio;
        int maxFaasletsPerFunction;

        // Threading
        std::string threadMode;
        int ompThreadPoolSize;

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
        int defaultMpiWorldSize;

        // Endpoint
        std::string endpointInterface;
        std::string endpointHost;
        int endpointPort;
        int endpointNumThreads;

        SystemConfig();

        void print();

        void reset();

    private:
        int getSystemConfIntParam(const char *name, const char *defaultValue);

        void initialise();
    };

    SystemConfig &getSystemConfig();
}