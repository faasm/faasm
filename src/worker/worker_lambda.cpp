#include "WorkerThreadPool.h"

#include <aws/aws.h>
#include <aws/lambda-runtime/runtime.h>

#include <util/logging.h>
#include <util/config.h>

#include <lambda/backend.h>

using namespace aws::lambda_runtime;


int main() {
    util::initLogging();
    
    faasm::initialiseLambdaBackend();

    util::SystemConfig &config = util::getSystemConfig();
    config.print();

    // Initialise worker pool
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
    logger->info("Starting thread pool with {} workers", config.threadsPerWorker);
    worker::WorkerThreadPool pool(config.threadsPerWorker);
    pool.start();

    auto handler_fn = [&logger](aws::lambda_runtime::invocation_request const &req) {
        logger->info("Received function call");

        // Receive JSON from invocation, decode into a message

        // Execute (pass to local scheduler)

        // Return a Lambda-friendly response
        const std::string output = faasm::getOutput();
        return invocation_response::success(
                output,
                "text/plain"
        );
    };

    logger->info("Listening for invocations");
    run_handler(handler_fn);

    return 0;
}

