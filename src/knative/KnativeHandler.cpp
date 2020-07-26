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

        // Very permissive CORS
        response.headers().add<Pistache::Http::Header::AccessControlAllowOrigin>("*");
        response.headers().add<Pistache::Http::Header::AccessControlAllowMethods>("GET,POST,PUT,OPTIONS");
        response.headers().add<Pistache::Http::Header::AccessControlAllowHeaders>("User-Agent,Content-Type");

        // Text response type
        response.headers().add<Pistache::Http::Header::ContentType>(
                Pistache::Http::Mime::MediaType("text/plain")
        );

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
            if (msg.isstatusrequest()) {
                scheduler::GlobalMessageBus &msgBus = scheduler::getGlobalMessageBus();
                responseStr = msgBus.getMessageStatus(msg.id());
            } else if(msg.isexecgraphrequest()) {
                scheduler::GlobalMessageBus &msgBus = scheduler::getGlobalMessageBus();

            } else if (msg.isflushrequest()) {
                const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
                logger->debug("Broadcasting flush request");

                message::Message flushMsg;
                flushMsg.set_isflushrequest(true);

                scheduler::SharingMessageBus &sharingBus = scheduler::SharingMessageBus::getInstance();
                sharingBus.broadcastMessage(flushMsg);
            } else {
                responseStr = executeFunction(msg);
            }
        }

        return responseStr;
    }
}
