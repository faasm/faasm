#include "WorkerThreadPool.h"

#include <aws/aws.h>
#include <aws/lambda-runtime/runtime.h>

#include <util/logging.h>
#include <util/config.h>
#include <util/json.h>

#include <lambda/backend.h>

using namespace aws::lambda_runtime;


int main() {
    util::initLogging();

    // Set things up
    faasm::initialiseLambdaBackend();

    util::SystemConfig &config = util::getSystemConfig();
    config.print();

    // Initialise worker pool
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
    logger->info("Initialising thread pool with {} workers", config.threadsPerWorker);
    worker::WorkerThreadPool pool(config.threadsPerWorker);
    logger->info("Thread pool initialised");

    // Start thread pool but detach so we can carry on invoking tasks from the main thread
    bool detach = true;
    pool.startThreadPool(detach);

    // Reference to local scheduler
    scheduler::Scheduler &sch = scheduler::getScheduler();
    scheduler::GlobalMessageBus &globalBus = scheduler::getGlobalMessageBus();

    auto handler_fn = [&logger, &sch, &globalBus](aws::lambda_runtime::invocation_request const &req) {
        // Receive JSON from invocation, decode into a message
        message::Message msg = util::jsonToMessage(req.payload);

        // Execute (pass to local scheduler)
        logger->info("Invoking {}", util::funcToString(msg));
        sch.callFunction(msg);

        // Wait for response
        message::Message result = globalBus.getFunctionResult(msg);
        std::string outputData = result.outputdata();

        // Return a Lambda-friendly response
        return invocation_response::success(
                outputData,
                "text/plain"
        );
    };

    logger->info("Calling Lambda runtime hook");
    run_handler_threads(handler_fn);

    // Clear up
    logger->info("Runtime function shutting down");
    faasm::tearDownLambdaBackend();

    return 0;
}
