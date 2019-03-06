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
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    // Set things up
    faasm::initialiseLambdaBackend();

    util::SystemConfig &config = util::getSystemConfig();
    config.print();

    auto handler_fn = [&logger, &config](aws::lambda_runtime::invocation_request const &req) {
        // Make sure this host gets added to the global worker set (happens in scheduler constructor)
        scheduler::Scheduler &sch = scheduler::getScheduler();

        // Initialise worker pool
        logger->info("Initialising thread pool with {} workers", config.threadsPerWorker);
        worker::WorkerThreadPool pool(config.threadsPerWorker);

        logger->info("Listening for requests for {}s", config.globalMessageTimeout);

        pool.startThreadPool(true);

        // Start global queue listener which will pass messages to the pool.
        // This call returns once there has been no message for the given timeout.
        bool isFinished = false;
        while(!isFinished) {
            pool.startGlobalQueueThread(false, true);

            // Try to get a global lock on execution. If we can get it, it implies nothing
            // is currently executing
            int executingCount = sch.getExecutingCount();
            isFinished = executingCount == 0;
            if(!isFinished) {
                logger->info("No new messages but still {} executing workers. Looping.", executingCount);
            }
        }

        // Here is the end of the invocation, we need to manually tidy up as we can't 
        // guarantee destructors will be called
        logger->info("Worker shutting down");
        faasm::tearDownLambdaBackend();

        // Remove this host from the global pool
        sch.removeHostFromGlobalSet();

        return invocation_response::success(
                "Worker finished",
                "text/plain"
        );
    };

    logger->info("Worker entering invocation loop");
    run_handler(handler_fn);

    return 0;
}
