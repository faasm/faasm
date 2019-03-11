#include "WorkerThreadPool.h"

#include <aws/aws.h>
#include <aws/lambda-runtime/runtime.h>

#include <util/json.h>
#include <wasm/FunctionLoader.h>

#include <lambda/backend.h>

#include <rapidjson/document.h>

using namespace aws::lambda_runtime;


int main() {
    util::initLogging();
    faasm::initialiseLambdaBackend();

    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    util::SystemConfig &config = util::getSystemConfig();
    config.print();

    auto handler_fn = [&config, &logger](aws::lambda_runtime::invocation_request const &req) {
        rapidjson::Document d;
        d.Parse(req.payload.c_str());

        std::string message;

        // Note, to ensure we have *exactly* the same underlying architecture
        // when generating machine code, we must make sure we run codegen in the
        // same function.
        if (d.HasMember("function")) {
            // Codegen
            message::Message msg = util::jsonToMessage(req.payload);
            logger->info("Generating object code for function {}", util::funcToString(msg));

            // Compile the function to object bytes
            wasm::FunctionLoader &functionLoader = wasm::getFunctionLoader();
            functionLoader.compileToObjectFile(msg);

            message = "Codegen finished";
        } else {
            // Ensure scheduler set up and this node is in global set
            scheduler::Scheduler &sch = scheduler::getScheduler();
            sch.addNodeToGlobalSet();

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
            message = "Worker finished";
        }

        return invocation_response::success(
                message,
                "text/plain"
        );
    };

    run_handler(handler_fn);

    faasm::tearDownLambdaBackend();

    return 0;
}
