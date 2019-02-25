#include "WorkerThreadPool.h"

#include <aws/aws.h>
#include <aws/lambda-runtime/runtime.h>

#include <util/logging.h>
#include <util/config.h>
#include <util/json.h>

#include <lambda/backend.h>

#include <thread>
#include <chrono>

using namespace aws::lambda_runtime;


int main() {
    util::initLogging();

    // Set things up
    faasm::initialiseLambdaBackend();

    util::SystemConfig &config = util::getSystemConfig();
    config.print();

    // Make sure this host gets added to the global worker set (happens in scheduler constructor)
    scheduler::getScheduler();

    // Initialise worker pool
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
    logger->info("Initialising thread pool with {} workers", config.threadsPerWorker);
    worker::WorkerThreadPool pool(config.threadsPerWorker);

    auto handler_fn = [&logger, &config, &pool](aws::lambda_runtime::invocation_request const &req) {
        logger->info("Listening for requests for {} seconds", config.lambdaWorkerTimeout);

        // Start global queue listener which will pass messages to the pool
        pool.startGlobalQueueThread();

        // TODO - put some kind of time limit on this
        // Start thread pool but don't detach from main thread
        pool.startThreadPool(false);

        return invocation_response::success(
                "Worker finished",
                "text/plain"
        );
    };

    logger->info("Worker entering invocation loop");
    run_handler(handler_fn);

    // Clear up
    // TODO - clear up thread pool properly here?
    logger->info("Worker shutting down");
    faasm::tearDownLambdaBackend();

    return 0;
}
