#include <aws/aws.h>
#include <aws/lambda-runtime/runtime.h>

#include <util/logging.h>
#include <util/config.h>
#include <util/json.h>
#include <util/func.h>
#include <wasm/FunctionLoader.h>

#include <lambda/backend.h>
#include <wasm/S3FunctionLoader.h>

using namespace aws::lambda_runtime;


int main() {
    util::initLogging();

    faasm::initialiseLambdaBackend();
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    util::SystemConfig &config = util::getSystemConfig();
    config.print();

    wasm::FunctionLoader &functionLoader = wasm::getFunctionLoader();

    auto handler_fn = [&logger, &functionLoader](aws::lambda_runtime::invocation_request const &req) {
        // Work out which function we're compiling
        message::Message msg = util::jsonToMessage(req.payload);
        logger->info("Generating object code for function {}", util::funcToString(msg));

        // Compile the function to object bytes
        functionLoader.compileToObjectFile(msg);

        // Return a Lambda-friendly response
        return invocation_response::success(
                "Code gen success",
                "text/plain"
        );
    };

    logger->info("Listening for function codegen calls");
    run_handler(handler_fn);

    // Tidy up
    logger->info("Codegen function shutting down");
    faasm::tearDownLambdaBackend();

    return 0;
}

