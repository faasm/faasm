#include "Redis.h"

#include <aws/aws.h>
#include <aws/lambda-runtime/runtime.h>

#include <util/logging.h>
#include <util/config.h>

#include <lambda/backend.h>

using namespace aws::lambda_runtime;


int main() {
    util::initLogging();
    faasm::initialiseLambdaBackend();

    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    util::SystemConfig &config = util::getSystemConfig();
    config.print();

    auto handler_fn = [&logger](aws::lambda_runtime::invocation_request const &req) {
        logger->info("Flushing Redis");

        redis::Redis &queue = redis::Redis::getQueue();
        redis::Redis &state = redis::Redis::getState();

        queue.flushAll();
        state.flushAll();

        return invocation_response::success(
                "Redis flushed",
                "text/plain"
        );
    };

    run_handler(handler_fn);

    faasm::tearDownLambdaBackend();

    return 0;
}
