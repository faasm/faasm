#include <wasm/WasmModule.h>

#include <redis/Redis.h>
#include <util/config.h>
#include <runner/timing.h>
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

    // Clear out redis
    redis::Redis &redis = redis::Redis::getQueue();
    redis.flushAll();

    // Set up the call
    std::string user = argv[1];
    std::string function = argv[2];
    message::Message call;
    call.set_user(user);
    call.set_function(function);
    int messageId = util::setMessageId(call);

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
    const util::TimePoint tp = runner::startTimer();
    scheduler::Scheduler &sch = scheduler::getScheduler();
    sch.callFunction(call);

    // Await the result
    scheduler::GlobalMessageBus &bus = scheduler::getGlobalMessageBus();
    const message::Message &result = bus.getFunctionResult(messageId);
    if(!result.success()) {
        throw std::runtime_error("Executing function failed");
    }

    runner::logEndTimer("Round trip execution", tp);

    pool.shutdown();

    return 0;
}