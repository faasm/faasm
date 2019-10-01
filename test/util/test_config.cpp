#include <catch/catch.hpp>

#include <util/config.h>
#include <util/environment.h>

using namespace util;

namespace tests {
    TEST_CASE("Test default system config initialisation", "[util]") {
        SystemConfig conf;
        conf.reset();

        REQUIRE(conf.threadsPerWorker == 5);

        // CI has to override some stuff
        if(conf.hostType == "ci") {
            REQUIRE(conf.redisStateHost == "redis");
            REQUIRE(conf.redisQueueHost == "redis");
        } else {
            REQUIRE(conf.redisStateHost == "localhost");
            REQUIRE(conf.redisQueueHost == "localhost");
            REQUIRE(conf.hostType == "default");
        }

        REQUIRE(conf.cgroupMode == "on");
        REQUIRE(conf.globalMessageBus == "redis");
        REQUIRE(conf.functionStorage == "local");
        REQUIRE(conf.fileserverUrl == "");
        REQUIRE(conf.serialisation == "json");
        REQUIRE(conf.bucketName == "");
        REQUIRE(conf.queueName == "faasm-messages");
        REQUIRE(conf.netNsMode == "off");
        REQUIRE(conf.awsLogLevel == "off");
        REQUIRE(conf.fsMode == "off");

        REQUIRE(conf.redisPort == "6379");

        REQUIRE(conf.maxNodes == 4);
        REQUIRE(conf.noScheduler == 0);
        REQUIRE(conf.prewarm == 1);
        REQUIRE(conf.maxQueueRatio == 3);
        REQUIRE(conf.maxWorkersPerFunction == 10);

        REQUIRE(conf.globalMessageTimeout == 60000);
        REQUIRE(conf.boundTimeout == 30000);
        REQUIRE(conf.unboundTimeout == 60000);

        REQUIRE(conf.stateStaleThreshold == 60000);
        REQUIRE(conf.stateClearThreshold == 300000);
        REQUIRE(conf.statePushInterval == 500);
        REQUIRE(conf.fullAsync == 0);
        REQUIRE(conf.fullSync == 0);
    }

    TEST_CASE("Test overriding system config initialisation", "[util]") {
        std::string originalHostType = getSystemConfig().hostType;

        std::string threads = setEnvVar("THREADS_PER_WORKER", "50");

        std::string hostType = setEnvVar("HOST_TYPE", "magic");
        std::string messageBus = setEnvVar("GLOBAL_MESSAGE_BUS", "blah");
        std::string funcStorage = setEnvVar("FUNCTION_STORAGE", "foobar");
        std::string fileserver = setEnvVar("FILESERVER_URL", "www.foo.com");
        std::string serialisation = setEnvVar("SERIALISATION", "proto");
        std::string bucket = setEnvVar("BUCKET_NAME", "foo-bucket");
        std::string queue = setEnvVar("QUEUE_NAME", "dummy-queue");
        std::string cgMode = setEnvVar("CGROUP_MODE", "off");
        std::string nsMode = setEnvVar("NETNS_MODE", "on");
        std::string awsLog = setEnvVar("AWS_LOG_LEVEL", "debug");
        std::string unsafe = setEnvVar("FS_MODE", "on");

        std::string redisState = setEnvVar("REDIS_STATE_HOST", "not-localhost");
        std::string redisQueue =setEnvVar("REDIS_QUEUE_HOST", "other-host");
        std::string redisPort =setEnvVar("REDIS_PORT", "1234");

        std::string irCacheMode = setEnvVar("IR_CACHE_MODE", "foo-ir-cache");

        std::string maxNodes =setEnvVar("MAX_NODES", "15");
        std::string noScheduler =setEnvVar("NO_SCHEDULER", "1");
        std::string prewarm =setEnvVar("PREWARM", "5");
        std::string queueRatio =setEnvVar("MAX_QUEUE_RATIO", "8888");
        std::string workers =setEnvVar("MAX_WORKERS_PER_FUNCTION", "7777");

        std::string globalTimeout =setEnvVar("GLOBAL_MESSAGE_TIMEOUT", "9876");
        std::string boundTimeout =setEnvVar("BOUND_TIMEOUT", "6666");
        std::string unboundTimeout =setEnvVar("UNBOUND_TIMEOUT", "5555");

        std::string staleThreshold =setEnvVar("STATE_STALE_THRESHOLD", "4444");
        std::string clearThreshold =setEnvVar("STATE_CLEAR_THRESHOLD", "3333");
        std::string pushInterval =setEnvVar("STATE_PUSH_INTERVAL", "2222");
        std::string fullAsync =setEnvVar("FULL_ASYNC", "1");
        std::string fullSync =setEnvVar("FULL_SYNC", "12");

        std::string funcDir = setEnvVar("FUNC_DIR","/tmp/foo");
        std::string pythonFuncDir = setEnvVar("PYTHON_FUNC_DIR","/tmp/par");
        std::string objDir = setEnvVar("OBJ_DIR", "/tmp/bar");
        std::string runtimeDir = setEnvVar("RUNTIME_FILES_DIR", "/tmp/rara");

        // Create new conf for test
        SystemConfig conf;
        REQUIRE(conf.threadsPerWorker == 50);

        REQUIRE(conf.hostType == "magic");
        REQUIRE(conf.globalMessageBus == "blah");
        REQUIRE(conf.functionStorage == "foobar");
        REQUIRE(conf.fileserverUrl == "www.foo.com");
        REQUIRE(conf.serialisation == "proto");
        REQUIRE(conf.bucketName == "foo-bucket");
        REQUIRE(conf.queueName == "dummy-queue");
        REQUIRE(conf.cgroupMode == "off");
        REQUIRE(conf.netNsMode == "on");
        REQUIRE(conf.awsLogLevel == "debug");
        REQUIRE(conf.fsMode == "on");

        REQUIRE(conf.redisStateHost == "not-localhost");
        REQUIRE(conf.redisQueueHost == "other-host");
        REQUIRE(conf.redisPort == "1234");

        REQUIRE(conf.irCacheMode == "foo-ir-cache");

        REQUIRE(conf.maxNodes == 15);
        REQUIRE(conf.noScheduler == 1);
        REQUIRE(conf.prewarm == 5);
        REQUIRE(conf.maxQueueRatio == 8888);
        REQUIRE(conf.maxWorkersPerFunction == 7777);

        REQUIRE(conf.globalMessageTimeout == 9876);
        REQUIRE(conf.boundTimeout == 6666);
        REQUIRE(conf.unboundTimeout == 5555);

        REQUIRE(conf.stateStaleThreshold == 4444);
        REQUIRE(conf.stateClearThreshold == 3333);
        REQUIRE(conf.statePushInterval == 2222);
        REQUIRE(conf.fullAsync == 1);
        REQUIRE(conf.fullSync == 12);

        REQUIRE(conf.functionDir == "/tmp/foo");
        REQUIRE(conf.pythonFunctionDir == "/tmp/par");
        REQUIRE(conf.objectFileDir == "/tmp/bar");
        REQUIRE(conf.runtimeFilesDir == "/tmp/rara");

        // Be careful with host type
        setEnvVar("HOST_TYPE", originalHostType);

        setEnvVar("THREADS_PER_WORKER", threads);

        setEnvVar("GLOBAL_MESSAGE_BUS", messageBus);
        setEnvVar("FUNCTION_STORAGE", funcStorage);
        setEnvVar("FILESERVER_URL", fileserver);
        setEnvVar("SERIALISATION", serialisation);
        setEnvVar("BUCKET_NAME", bucket);
        setEnvVar("QUEUE_NAME", queue);
        setEnvVar("CGROUP_MODE", cgMode);
        setEnvVar("NETNS_MODE", nsMode);
        setEnvVar("AWS_LOG_LEVEL", awsLog);
        setEnvVar("FS_MODE", unsafe);

        setEnvVar("REDIS_STATE_HOST", redisState);
        setEnvVar("REDIS_QUEUE_HOST", redisQueue);
        setEnvVar("REDIS_PORT", redisPort);

        setEnvVar("IR_CACHE_MODE", irCacheMode);

        setEnvVar("MAX_NODES", maxNodes);
        setEnvVar("NO_SCHEDULER", noScheduler);
        setEnvVar("PREWARM", prewarm);
        setEnvVar("MAX_QUEUE_RATIO", queueRatio);
        setEnvVar("MAX_WORKERS_PER_FUNCTION", workers);

        setEnvVar("GLOBAL_MESSAGE_TIMEOUT", globalTimeout);
        setEnvVar("BOUND_TIMEOUT", boundTimeout);
        setEnvVar("UNBOUND_TIMEOUT", unboundTimeout);

        setEnvVar("STATE_STALE_THRESHOLD", staleThreshold);
        setEnvVar("STATE_CLEAR_THRESHOLD", clearThreshold);
        setEnvVar("STATE_PUSH_INTERVAL", pushInterval);
        setEnvVar("FULL_ASYNC", fullAsync);
        setEnvVar("FULL_SYNC", fullSync);

        setEnvVar("FUNC_DIR", funcDir);
        setEnvVar("PYTHON_FUNC_DIR", pythonFuncDir);
        setEnvVar("OBJ_DIR", objDir);
        setEnvVar("RUNTIME_FILES_DIR", runtimeDir);
    }

    TEST_CASE("Check we can't have both full sync and full async on at the same time", "[conf]") {
        setEnvVar("FULL_ASYNC", "1");
        setEnvVar("FULL_SYNC", "1");

        REQUIRE_THROWS(SystemConfig());

        unsetEnvVar("FULL_ASYNC");
        unsetEnvVar("FULL_SYNC");
    }
}