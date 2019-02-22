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

    wasm::S3FunctionLoader loader;

    auto handler_fn = [&logger, &loader](aws::lambda_runtime::invocation_request const &req) {
        // Work out which function we're compiling
        message::Message msg = util::jsonToMessage(req.payload);
        logger->info("Generating object code for function {}", util::funcToString(msg));

        // Compile the function to object bytes
        loader.compileToObjectFile(msg);

        // Return a Lambda-friendly response
        const std::string output = faasm::getOutput();
        return invocation_response::success(
                output,
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

