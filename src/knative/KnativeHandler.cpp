#include "KnativeHandler.h"

#include <util/logging.h>
#include <util/timing.h>
#include <util/json.h>
#include <scheduler/Scheduler.h>

using namespace Pistache;

namespace knative {
    KnativeHandler::KnativeHandler() : globalBus(scheduler::getGlobalMessageBus()), conf(util::getSystemConfig()) {

    }

    void KnativeHandler::onRequest(const Http::Request &request, Http::ResponseWriter response) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Received request");

        PROF_START(knativeRoundTrip)

        // Parse message from JSON in request
        const std::string requestStr = request.body();
        if (requestStr.empty()) {
            response.send(Http::Code::Bad_Request, "Must provide user/ function as JSON");
        }

        // Parse the message
        message::Message msg = util::jsonToMessage(requestStr);
        util::setMessageId(msg);

        // Schedule it
        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.callFunction(msg);

        // Await result on global bus (may have been executed on a different worker)
        const message::Message result = globalBus.getFunctionResult(msg.id(), conf.globalMessageTimeout);

        response.send(Http::Code::Ok, result.outputdata() + "\n");

        PROF_END(knativeRoundTrip)
    }
}
