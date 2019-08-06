#include <worker/WorkerThreadPool.h>

#include <aws/aws.h>
#include <aws/lambda-runtime/runtime.h>

#include <util/json.h>
#include <storage/FunctionLoader.h>
#include <wasm/codegen.h>

#include <rapidjson/document.h>

using namespace aws::lambda_runtime;

#define PYTHON_RUNTIME_DIRECTORY "/usr/local/faasm/runtime_root/lib/python3.7"

int main() {
    util::initLogging();
    awswrapper::initSDK();

    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    util::SystemConfig &config = util::getSystemConfig();
    config.print();

    auto handler_fn = [&config, &logger](aws::lambda_runtime::invocation_request const &req) {
        rapidjson::Document d;
        d.Parse(req.payload.c_str());

        std::string message;

        // Work out what sort of invocation is being requested
        std::string target;
        if (d.HasMember("target")) {
            target = d["target"].GetString();

            // Note, to ensure we have *exactly* the same underlying architecture
            // when generating machine code, we must make sure we run codegen in the
            // same function.
            if (target == "func-codegen") {
                message::Message msg = util::jsonToMessage(req.payload);
                logger->info("Generating object code for function {}", util::funcToString(msg));

                // Compile the function to object bytes
                storage::FunctionLoader &functionLoader = storage::getFunctionLoader();
                functionLoader.compileToObjectFile(msg);

                message = "Codegen finished";
            } else if (target == "python-codegen") {
                message::Message msg = util::jsonToMessage(req.payload);
                logger->info("Generating object code for directory {}", PYTHON_RUNTIME_DIRECTORY);

                // Run the codegen on the Python directory
                wasm::codeGenForDir(PYTHON_RUNTIME_DIRECTORY);

                message = "Python codegen finished";
            } else {
                logger->error("Unrecognised target: {}", target);
                return invocation_response::failure(
                        "Unrecognised worker request type",
                        "text/plain"
                );
            }
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

    awswrapper::cleanUpSDK();

    return 0;
}
