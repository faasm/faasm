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
    faasm::initialiseLambdaBackend();

    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    util::SystemConfig &config = util::getSystemConfig();
    config.print();

    auto handler_fn = [&config, &logger](aws::lambda_runtime::invocation_request const &req) {
        scheduler::Scheduler &sch = scheduler::getScheduler();

        // Initialise worker pool
        logger->info("Initialising thread pool with {} workers", config.threadsPerWorker);
        worker::WorkerThreadPool pool(config.threadsPerWorker);

        // Start up the thread pool
        logger->info("Listening for requests for {}ms", config.globalMessageTimeout);

        pool.startThreadPool();

        // Work sharing thread
        // pool.startSharingThread();

        // State management thread
        // pool.startStateThread();

        pool.startGlobalQueueThread();

        logger->info("Removing node from global set");
        sch.clear();

        // Wait for the pool to properly shut down
        pool.shutdown();

        logger->info("Returning Lambda response");
        std::string message = "Worker finished";

        return invocation_response::success(
                message,
                "text/plain"
        );
    };

    run_handler(handler_fn);

    faasm::tearDownLambdaBackend();

    return 0;
}
