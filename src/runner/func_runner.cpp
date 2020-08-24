#include <wasm/WasmModule.h>

#include <faabric/redis/Redis.h>
#include <faabric/util/config.h>
#include <faabric/util/timing.h>
#include <faaslet/FaasletPool.h>


int main(int argc, char *argv[]) {
    faabric::utilinitLogging();
    const std::shared_ptr<spdlog::logger> logger = faabric::utilgetLogger();

    if (argc < 3) {
        logger->error("Must provide user and function name");
        return 1;
    }

    faabric::utilSystemConfig &conf = faabric::utilgetSystemConfig();

    // Set short timeouts to die quickly
    conf.boundTimeout = 60000;
    conf.unboundTimeout = 60000;
    conf.globalMessageTimeout = 60000;
    conf.chainedCallTimeout = 60000;

    // Make sure we have enough space for chained calls
    int nThreads = 10;
    conf.defaultMpiWorldSize = 5;
    conf.maxFaaslets = nThreads;
    conf.maxFaasletsPerFunction = nThreads;

    // Clear out redis
    redis::Redis &redis = redis::Redis::getQueue();
    redis.flushAll();

    // Set up the call
    std::string user = argv[1];
    std::string function = argv[2];
    faabric::Message call = faabric::utilmessageFactory(user, function);

    if (user == "ts") {
        call.set_istypescript(true);
    }

    if (user == "python") {
        faabric::utilconvertMessageToPython(call);
        logger->info("Running Python function {}/{}", call.pythonuser(), call.pythonfunction());
    } else {
        logger->info("Running function {}/{}", user, function);
    }

    if (argc > 3) {
        std::string inputData = argv[3];
        call.set_inputdata(inputData);

        logger->info("Adding input data: {}", inputData);
    }

    // Set up a worker pool
    faaslet::FaasletPool pool(nThreads);

    // Start the workers
    pool.startThreadPool();

    // Submit the invocation
    PROF_START(roundTrip)
    scheduler::Scheduler &sch = scheduler::getScheduler();
    sch.callFunction(call);

    // Await the result
    const faabric::Message &result = sch.getFunctionResult(call.id(), conf.globalMessageTimeout);
    if (result.returnvalue() != 0) {
        logger->error("Execution failed: {}", result.outputdata());
        throw std::runtime_error("Executing function failed");
    }

    PROF_END(roundTrip)

    pool.shutdown();

    return 0;
}