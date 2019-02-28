#include "Redis.h"

#include <aws/aws.h>
#include <aws/lambda-runtime/runtime.h>

#include <util/logging.h>
#include <util/config.h>

#include <lambda/backend.h>

#include "rapidjson/document.h"

using namespace aws::lambda_runtime;


int main() {
    util::initLogging();
    faasm::initialiseLambdaBackend();

    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    util::SystemConfig &config = util::getSystemConfig();
    config.print();

    auto handler_fn = [&logger](aws::lambda_runtime::invocation_request const &req) {
        rapidjson::Document d;
        d.Parse(req.payload.c_str());

        std::string target = "all";
        if (d.HasMember("target")) {
            target = d["target"].GetString();
        }

        logger->info("Flushing Redis target {}", target);

        redis::Redis &queue = redis::Redis::getQueue();
        redis::Redis &state = redis::Redis::getState();

        if (target == "all") {
            queue.flushAll();
            state.flushAll();
        } else if (target == "queue") {
            queue.flushAll();
        } else {
            logger->error("Unrecognised target: {}", target);
        }

        return invocation_response::success(
                "Redis operation done",
                "text/plain"
        );
    };

    run_handler(handler_fn);

    faasm::tearDownLambdaBackend();

    return 0;
}
