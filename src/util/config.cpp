#include "config.h"
#include "environment.h"
#include "logging.h"
#include "random.h"


namespace util {
    SystemConfig &getSystemConfig() {
        static SystemConfig conf;
        return conf;
    }

    SystemConfig::SystemConfig() {
        this->initialise();
    }

    void SystemConfig::initialise() {
        // TODO - max cannot exceed the underlying number of available namespaces. Good to decouple?
        threadsPerWorker = this->getSystemConfIntParam("THREADS_PER_WORKER", "5");

        // System
        hostType = getEnvVar("HOST_TYPE", "default");
        globalMessageBus = getEnvVar("GLOBAL_MESSAGE_BUS", "redis");
        functionStorage = getEnvVar("FUNCTION_STORAGE", "local");
        fileserverUrl = getEnvVar("FILESERVER_URL", "");
        serialisation = getEnvVar("SERIALISATION", "json");
        bucketName = getEnvVar("BUCKET_NAME", "");
        queueName = getEnvVar("QUEUE_NAME", "faasm-messages");
        cgroupMode = getEnvVar("CGROUP_MODE", "on");
        netNsMode = getEnvVar("NETNS_MODE", "off");
        logLevel = getEnvVar("LOG_LEVEL", "info");
        awsLogLevel = getEnvVar("AWS_LOG_LEVEL", "off");
        unsafeMode = getEnvVar("UNSAFE_MODE", "off");

        // Redis
        redisStateHost = getEnvVar("REDIS_STATE_HOST", "localhost");
        redisQueueHost = getEnvVar("REDIS_QUEUE_HOST", "localhost");
        redisPort = getEnvVar("REDIS_PORT", "6379");

        // Scheduling
        maxNodes = this->getSystemConfIntParam("MAX_NODES", "4");
        noScheduler = this->getSystemConfIntParam("NO_SCHEDULER", "0");
        prewarm = this->getSystemConfIntParam("PREWARM", "1");
        maxQueueRatio = this->getSystemConfIntParam("MAX_QUEUE_RATIO", "3");
        maxWorkersPerFunction = this->getSystemConfIntParam("MAX_WORKERS_PER_FUNCTION", "10");

        // Worker-related timeouts (all in seconds)
        globalMessageTimeout = this->getSystemConfIntParam("GLOBAL_MESSAGE_TIMEOUT", "60000");
        boundTimeout = this->getSystemConfIntParam("BOUND_TIMEOUT", "30000");
        unboundTimeout = this->getSystemConfIntParam("UNBOUND_TIMEOUT", "60000");

        // State timeouts (all in millis)
        stateStaleThreshold = this->getSystemConfIntParam("STATE_STALE_THRESHOLD", "60000");
        stateClearThreshold = this->getSystemConfIntParam("STATE_CLEAR_THRESHOLD", "300000");
        statePushInterval = this->getSystemConfIntParam("STATE_PUSH_INTERVAL", "500");
        fullAsync = this->getSystemConfIntParam("FULL_ASYNC", "0");
        fullSync = this->getSystemConfIntParam("FULL_SYNC", "0");

        if(fullAsync == 1 && fullSync == 1) {
            throw std::runtime_error("Can't have both full async and full sync on");
        }
    }

    int SystemConfig::getSystemConfIntParam(const char *name, const char *defaultValue) {
        int value = stoi(getEnvVar(name, defaultValue));

        return value;
    };

    void SystemConfig::reset() {
        this->initialise();
    }

    void SystemConfig::print() {
        const std::shared_ptr<spdlog::logger> &logger = getLogger();

        logger->info("--- System ---");
        logger->info("HOST_TYPE                  {}", hostType);
        logger->info("GLOBAL_MESSAGE_BUS         {}", globalMessageBus);
        logger->info("FUNCTION_STORAGE           {}", functionStorage);
        logger->info("FILESERVER_URL             {}", fileserverUrl);
        logger->info("SERIALISATION              {}", serialisation);
        logger->info("BUCKET_NAME                {}", bucketName);
        logger->info("QUEUE_NAME                 {}", queueName);
        logger->info("CGROUP_MODE                {}", cgroupMode);
        logger->info("NETNS_MODE                 {}", netNsMode);
        logger->info("LOG_LEVEL                  {}", logLevel);
        logger->info("AWS_LOG_LEVEL              {}", awsLogLevel);
        logger->info("UNSAFE_MODE                {}", unsafeMode);

        logger->info("--- Redis ---");
        logger->info("REDIS_STATE_HOST           {}", redisStateHost);
        logger->info("REDIS_QUEUE_HOST           {}", redisQueueHost);
        logger->info("REDIS_PORT                 {}", redisPort);

        logger->info("--- Scheduling ---");
        logger->info("MAX_NODES                  {}", maxNodes);
        logger->info("THREADS_PER_WORKER         {}", threadsPerWorker);
        logger->info("NO_SCHEDULER               {}", noScheduler);
        logger->info("PREWARM                    {}", prewarm);
        logger->info("MAX_QUEUE_RATIO            {}", maxQueueRatio);
        logger->info("MAX_WORKERS_PER_FUNCTION   {}", maxWorkersPerFunction);

        logger->info("--- Timeouts ---");
        logger->info("GLOBAL_MESSAGE_TIMEOUT     {}", globalMessageTimeout);
        logger->info("BOUND_TIMEOUT              {}", boundTimeout);
        logger->info("UNBOUND_TIMEOUT            {}", unboundTimeout);

        logger->info("--- State ---");
        logger->info("STATE_STALE_THRESHOLD      {}", stateStaleThreshold);
        logger->info("STATE_CLEAR_THRESHOLD      {}", stateClearThreshold);
        logger->info("STATE_PUSH_INTERVAL        {}", statePushInterval);
        logger->info("FULL_ASYNC                 {}", fullAsync);
        logger->info("FULL_SYNC                  {}", fullSync);
    }

    std::string getNodeId() {
        static std::string nodeId;

        if(nodeId.empty()) {
            // Generate random node ID
            nodeId = util::randomString(20);
        }

        return nodeId;
    }
}
