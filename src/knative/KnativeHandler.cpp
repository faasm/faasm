#include "KnativeHandler.h"

#include <util/logging.h>
#include <util/timing.h>
#include <util/json.h>
#include <scheduler/Scheduler.h>

namespace knative {
    KnativeHandler::KnativeHandler() : conf(util::getSystemConfig()) {

    }

    void KnativeHandler::onTimeout(const Pistache::Http::Request &request, Pistache::Http::ResponseWriter writer) {
        writer.send(Pistache::Http::Code::No_Content);
    }

    void KnativeHandler::onRequest(const Pistache::Http::Request &request, Pistache::Http::ResponseWriter response) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Knative handler received request");

        PROF_START(knativeRoundTrip)

        // Set response timeout
        response.timeoutAfter(std::chrono::milliseconds(conf.globalMessageTimeout));

        // Parse message from JSON in request
        const std::string requestStr = request.body();
        std::string responseStr = handleFunction(requestStr);

        PROF_END(knativeRoundTrip)
        response.send(Pistache::Http::Code::Ok, responseStr);
    }

    std::string KnativeHandler::handleFunction(const std::string &requestStr) {
        std::string responseStr;
        if (requestStr.empty()) {
            responseStr = "Empty request";
        } else {
            message::Message msg = util::jsonToMessage(requestStr);
            responseStr = executeFunction(msg);
        }

        return responseStr;
    }
}
