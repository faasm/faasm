#include "KnativeNativeHandler.h"

extern "C" {
#include <emulator/emulator_api.h>
}

#include <emulator/emulator.h>

#include <faasm/core.h>
#include <util/logging.h>
#include <util/json.h>
#include <util/environment.h>
#include <util/func.h>
#include <scheduler/Scheduler.h>
#include <shared_mutex>

#include <utility>


namespace knative_native {
    std::shared_mutex coldMx;

    KnativeNativeHandler::KnativeNativeHandler(
            std::string userIn,
            std::string funcIn
    ) : user(std::move(userIn)), func(std::move(funcIn)) {

        isCold = true;
    }

    void KnativeNativeHandler::onRequest(
            const Pistache::Http::Request &request,
            Pistache::Http::ResponseWriter response
    ) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Simulate cold start if this is first request
        if (isCold) {
            util::FullLock lock(coldMx);

            if (isCold) {
                logger->info("Simulating cold start");

                std::string coldStartStr = util::getEnvVar("COLD_START_DELAY_MS", "0");
                long coldStartMs = std::stol(coldStartStr);
                usleep(coldStartMs * 1000);
                isCold = false;
            }
        } else {
            logger->info("Warm start");
        }

        // Set up the function
        const std::string requestStr = request.body();
        message::Message msg = util::jsonToMessage(requestStr);
        unsigned int messageId = setEmulatedMessage(msg);

        // Make sure the message ID is set for potentially passing into a thread
        msg.set_id(messageId);

        logger->debug("Knative native request: {}", requestStr);

        std::string outputStr;
        if (msg.isasync()) {
            logger->debug("Executing function index {} async", msg.idx());

            // Execute async message in detached thread
            std::thread t([msg] {
                // Set up message in this thread
                setEmulatedMessage(msg);

                // Run it
                exec(msg.idx());
                emulatorSetCallStatus(1);
            });

            t.detach();

            outputStr = util::buildAsyncResponse(msg);
        } else {
            logger->debug("Executing function index {} sync", msg.idx());
            exec(msg.idx());

            // Get the output
            outputStr = getEmulatorOutputDataString();
        }

        // Make sure we flush stdout
        fflush(stdout);

        response.send(Pistache::Http::Code::Ok, outputStr);
    }
}
