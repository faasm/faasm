#include <wasm/WasmModule.h>

#include <redis/Redis.h>
#include <util/config.h>
#include <util/timing.h>
#include <worker/WorkerThreadPool.h>


int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> logger = util::getLogger();

    if (argc < 3) {
        logger->error("Must provide user and function name");
        return 1;
    }

    util::SystemConfig &conf = util::getSystemConfig();
    conf.unsafeMode = "on";

    // Set short timeouts to die quickly
    conf.boundTimeout = 3000;
    conf.unboundTimeout = 3000;

    // Custom conf here to specify always scale out but put a limit on the workers
    conf.maxQueueRatio = 0;
    conf.maxWorkersPerFunction = 6;

    // Clear out redis
    redis::Redis &redis = redis::Redis::getQueue();
    redis.flushAll();

    // Set up the call
    std::string user = argv[1];
    std::string function = argv[2];
    message::Message call = util::messageFactory(user, function);

    if(user == "ts") {
        call.set_istypescript(true);
    }

    logger->info("Running function {}/{}", user, function);

    if (argc > 3) {
        std::string inputData = argv[3];
        call.set_inputdata(inputData);

        logger->info("Adding input data: {}", inputData);
    }

    // Set up a worker pool
    worker::WorkerThreadPool pool(4);
    pool.startThreadPool();

    // Submit the invocation
    PROF_START(roundTrip)
    scheduler::Scheduler &sch = scheduler::getScheduler();
    sch.callFunction(call);

    // Await the result
    scheduler::GlobalMessageBus &bus = scheduler::getGlobalMessageBus();
    const message::Message &result = bus.getFunctionResult(call.id(), 20000);
    if(!result.success()) {
        throw std::runtime_error("Executing function failed");
    }

    PROF_END(roundTrip)

    pool.shutdown();

    return 0;
}