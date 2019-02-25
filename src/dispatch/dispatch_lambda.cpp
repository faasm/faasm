#include <aws/aws.h>
#include <aws/lambda-runtime/runtime.h>

#include <util/logging.h>
#include <util/config.h>
#include <util/json.h>
#include <util/func.h>
#include <scheduler/GlobalMessageBus.h>
#include <scheduler/Scheduler.h>

#include <lambda/backend.h>
#include <aws/LambdaWrapper.h>


using namespace aws::lambda_runtime;


int main() {
    util::initLogging();

    faasm::initialiseLambdaBackend();
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    util::SystemConfig &config = util::getSystemConfig();
    config.print();

    scheduler::GlobalMessageBus &globalBus = scheduler::getGlobalMessageBus();
    redis::Redis &redis = redis::Redis::getQueue();
    awswrapper::LambdaWrapper &lambda = awswrapper::LambdaWrapper::getThreadLocal();
    
    auto handler_fn = [&logger, &globalBus, &redis, &lambda](aws::lambda_runtime::invocation_request const &req) {
        // Get the function
        message::Message msg = util::jsonToMessage(req.payload);
        logger->info("Queueing request to {}", util::funcToString(msg));

        // Dispatch function
        globalBus.enqueueMessage(msg);

        // Invoke lambda function asynchronously if no workers present
        // (to ensure there's something there to process messages)
        long workerCount = redis.scard(GLOBAL_WORKER_SET);
        if(workerCount == 0) {
            logger->info("No workers currently registered. Spawning one");

            lambda.invokeFunction("faasm-worker", "", false);
        } else {
            logger->info("{} workers currently registered. Not requesting another", workerCount);
        }

        // Handle sync/ async Faasm requests accordingly
        std::string resultData;
        if (msg.isasync()) {
            logger->info("Async request {}", util::funcToString(msg));
            resultData = "Async request submitted";
        } else {
            logger->info("Sync request {}", util::funcToString(msg));
            message::Message result = globalBus.getFunctionResult(msg);

            logger->info("Finished request {}", util::funcToString(msg));

            resultData = result.outputdata();
        }

        // Return a Lambda-friendly response
        return invocation_response::success(
                resultData,
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

