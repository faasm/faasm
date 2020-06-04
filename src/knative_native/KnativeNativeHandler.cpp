#include "KnativeNativeHandler.h"

extern "C" {
#include <emulator/emulator_api.h>
}

#include <emulator/emulator.h>

#include <random>

#include <faasm/core.h>
#include <util/logging.h>
#include <util/json.h>
#include <util/environment.h>
#include <util/func.h>
#include <scheduler/Scheduler.h>
#include <shared_mutex>

#include <utility>
#include <state/State.h>


namespace knative_native {
    // Global request counter
    std::atomic<int> requestCount = 0;

    // Random generators
    std::random_device rd{};
    std::mt19937 randGenerator{rd()};
    std::normal_distribution<double> normalDist{1.0,0.5};

    KnativeNativeHandler::KnativeNativeHandler(
            std::string userIn,
            std::string funcIn
    ) : user(std::move(userIn)), func(std::move(funcIn)) {
    }

    void KnativeNativeHandler::onRequest(
            const Pistache::Http::Request &request,
            Pistache::Http::ResponseWriter response
    ) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // See of cold starts have been requested
        bool coldStartRequired = false;
        const std::string requestStr = request.body();
        std::string coldStartIntervalStr = util::getValueFromJsonString("cold_start_interval", requestStr);
        if(!coldStartIntervalStr.empty()) {
            int coldStartInterval = std::stoi(coldStartIntervalStr);
            coldStartRequired = requestCount % coldStartInterval == 0;
        }

        // Simulate a cold start if necessary
        if (coldStartRequired || requestCount == 0) {
            // Clear out state locally
            state::State &s = state::getGlobalState();
            s.forceClearAll(false);

            // Do the sleep
            std::string coldStartDelayStr = util::getEnvVar("COLD_START_DELAY_MS", "0");
            if (coldStartDelayStr != "0") {
                // Random cold start around requested delay
                int maxDelay = std::stoi(coldStartDelayStr);
                double multiplier = normalDist(randGenerator);
                int coldStartMs = (int) std::round(multiplier * maxDelay);
                coldStartMs = std::max(0, coldStartMs);

                logger->info("Cold start {}ms ({}th request, {} interval)", coldStartMs, requestCount, coldStartIntervalStr);
                usleep(coldStartMs * 1000);
            }
        } else {
            logger->info("Warm start ({}th request, {} interval)", requestCount, coldStartIntervalStr);
        }

        // Up the request count
        requestCount++;

        // Set up the function
        message::Message msg = util::jsonToMessage(requestStr);
        unsigned int messageId = setEmulatedMessage(msg);

        // Make sure the message ID is set for potentially passing into a thread
        msg.set_id(messageId);

        logger->debug("Native request: {}", requestStr);

        std::string outputStr;
        if (msg.isasync()) {
            logger->debug("Executing function index {} async", msg.idx());

            // Execute async message in detached thread
            std::thread t([msg] {
                // Set up message in this thread
                setEmulatedMessage(msg);

                // Run it
                exec(msg.idx());
                emulatorSetCallStatus(true);
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
