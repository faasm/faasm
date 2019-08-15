#include "KnativeHandler.h"

#include <util/logging.h>
#include <scheduler/Scheduler.h>

using namespace Pistache;

namespace knative {
    void KnativeHandler::onRequest(const Http::Request &request, Http::ResponseWriter response) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Received request");

        // TODO - parse message from JSON in request
        message::Message msg;

        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.callFunction(msg);

        // TODO - return appropriate message here
        response.send(Http::Code::Ok, "");
    }
}
