#include <util/logging.h>
#include <util/config.h>
#include <fstream>
#include <util/func.h>

#include <redis/Redis.h>
#include <worker/WorkerThreadPool.h>


int main(int argc, char *argv[]) {
    util::initLogging();
    const std::shared_ptr <spdlog::logger> &logger = util::getLogger();

    if (argc < 2) {
        logger->error("Usage:\nprk_local <kernel>");
        return 1;
    }

    // Get the kernel and cmdline args
    std::string kernel = argv[1];
    std::string cmdline;
    if(kernel == "nstream") {
        cmdline = "2 200 0";   // iterations, vector length, offset
    } else {
        logger->error("Unrecognised kernel: {}", kernel);
        return 1;
    }

    // Set up the call
    message::Message msg = util::messageFactory("prk", kernel);
    msg.set_cmdline(cmdline);

    // Set up config
    util::SystemConfig &conf = util::getSystemConfig();
    int nThreads = 6;
    conf.defaultMpiWorldSize = 5;
    conf.threadsPerWorker = nThreads;
    conf.maxWorkersPerFunction = nThreads;
    conf.boundTimeout = 60000;
    conf.unboundTimeout = 60000;
    conf.globalMessageTimeout = 60000;
    conf.chainedCallTimeout = 60000;

    // Clear out redis
    redis::Redis::getQueue().flushAll();

    // Set up a worker pool
    worker::WorkerThreadPool pool(nThreads);
    pool.startMpiThread();
    pool.startThreadPool();

    // Invoke
    scheduler::Scheduler &sch = scheduler::getScheduler();
    sch.callFunction(msg);

    // Await the result
    scheduler::GlobalMessageBus &bus = scheduler::getGlobalMessageBus();
    const message::Message &result = bus.getFunctionResult(msg.id(), conf.globalMessageTimeout);
    if (!result.success()) {
        logger->error("Execution failed: {}", result.outputdata());
        throw std::runtime_error("Executing function failed");
    }

    pool.shutdown();

    return 0;
}