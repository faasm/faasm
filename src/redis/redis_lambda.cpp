#include "Redis.h"

#include <aws/aws.h>
#include <aws/lambda-runtime/runtime.h>

#include <util/logging.h>
#include <util/config.h>

#include <lambda/backend.h>
#include <scheduler/Scheduler.h>

#include <rapidjson/document.h>

using namespace aws::lambda_runtime;


int main() {
    util::initLogging();
    awswrapper::initSDK();

    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    util::SystemConfig &config = util::getSystemConfig();
    config.print();

    auto handler_fn = [&logger](aws::lambda_runtime::invocation_request const &req) {
        rapidjson::Document d;
        d.Parse(req.payload.c_str());

        std::string target = "all";
        if (d.HasMember("target")) {
            target = d["target"].GetString();
        } else {
            return invocation_response::success(
                    "Must provide a target for the function",
                    "text/plain"
            );
        }

        logger->info("Handling Redis target {}", target);

        redis::Redis &queue = redis::Redis::getQueue();
        redis::Redis &state = redis::Redis::getState();

        std::string message = "Target complete: " + target;

        if (target == "flush-all") {
            queue.flushAll();
            state.flushAll();
        } else if (target == "flush-queue") {
            queue.flushAll();
        } else if (target == "worker-count") {
            long count = queue.scard(GLOBAL_NODE_SET);
            message = "Worker count: " + std::to_string(count);
        } else {
            message = "Unrecognised target: " + target;
            logger->error(message);
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
