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

    scheduler::Scheduler &sch = scheduler::getScheduler();

    // Initialise worker pool
    logger->info("Initialising thread pool with {} workers", config.threadsPerWorker);
    worker::WorkerThreadPool pool(config.threadsPerWorker);

    auto handler_fn = [&config, &logger, &pool, &sch](aws::lambda_runtime::invocation_request const &req) {
        // Start up the thread pool
        logger->info("Listening for requests for {}s", config.globalMessageTimeout);
        pool.startThreadPool(false);

        // Make sure this host is in the global set
        sch.addHostToGlobalSet();

        // Listen for messages on the global queue.
        // This will pass messages to the pool.
        // This loop will break returns once there has been no message for the given timeout
        // *provided* no threads are still executing
        bool isFinished = false;
        while(!isFinished) {
            pool.startGlobalQueueThread(true, true);

            // Try to get a global lock on execution. If we can get it, it implies nothing
            // is currently executing
            int executingCount = sch.getExecutingCount();
            isFinished = executingCount == 0;
            if(!isFinished) {
                logger->info("No new messages but still {} executing workers. Looping.", executingCount);
            }
        }

        logger->info("Removing host from global set");
        sch.removeHostFromGlobalSet();

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
