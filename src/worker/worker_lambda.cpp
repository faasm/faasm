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

    // Start thread pool and detach from main thread
    bool detach = true;
    pool.startThreadPool(detach);
    logger->info("Thread pool started", config.threadsPerWorker);

    // Start global queue listener which will pass messages to the pool
    pool.startGlobalQueueThread();

    auto handler_fn = [&logger, &config](aws::lambda_runtime::invocation_request const &req) {
        logger->info("Listening for requests for {} seconds", config.lambdaWorkerTimeout);

        // Sleep for a while as threads execute in background
        std::this_thread::sleep_for(std::chrono::seconds(config.lambdaWorkerTimeout));
        logger->info("Worker finished, threads pausing");

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
