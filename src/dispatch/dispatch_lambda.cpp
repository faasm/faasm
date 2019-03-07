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

    scheduler::Scheduler &sch = scheduler::getScheduler();
    scheduler::GlobalMessageBus &globalBus = scheduler::getGlobalMessageBus();

    auto handler_fn = [&logger, &globalBus, &sch](aws::lambda_runtime::invocation_request const &req) {
        // Get the function
        message::Message msg = util::jsonToMessage(req.payload);
        logger->info("Queueing request to {}", util::funcToString(msg));
        util::addResultKeyToMessage(msg);

        // Dispatch function
        globalBus.enqueueMessage(msg);

        // Invoke lambda function asynchronously if no workers present
        // (to ensure there's something there to process messages)
        long workerCount = sch.getGlobalSetSize();
        if(workerCount == 0) {
            logger->info("No workers currently registered. Spawning one");

            sch.scaleOut(1);
        } else {
            logger->info("{} workers currently registered. Not requesting another", workerCount);
        }

        // Handle sync/ async Faasm requests accordingly
        std::string resultData = "Function dispatched";
        if (msg.isasync()) {
            logger->info("Async request {}", util::funcToString(msg));
            resultData = "Async request submitted";
        } else {
            logger->info("Sync request {}", util::funcToString(msg));
            message::Message result = globalBus.getFunctionResult(msg);

            logger->info("Finished request {}", util::funcToString(msg));

            resultData = result.outputdata();
        }

        return invocation_response::success(
                resultData,
                "text/plain"
        );
    };

    logger->info("Listening for function dispatch calls");
    run_handler(handler_fn);

    // Tidy up
    logger->info("Shutting down");
    faasm::tearDownLambdaBackend();

    return 0;
}

