#include "KnativeHandler.h"

#include <util/logging.h>
#include <util/timing.h>
#include <util/json.h>
#include <scheduler/Scheduler.h>

namespace knative {
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
        util::SystemConfig &conf = util::getSystemConfig();
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
            scheduler::Scheduler &sched = scheduler::getScheduler();

            if (msg.isstatusrequest()) {
                responseStr = sched.getMessageStatus(msg.id());

            } else if(msg.isexecgraphrequest()) {
                scheduler::ExecGraph execGraph = sched.getFunctionExecGraph(msg.id());
                responseStr = scheduler::execGraphToJson(execGraph);

            } else if (msg.isflushrequest()) {
                const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
                logger->debug("Broadcasting flush request");

                sched.broadcastFlush(msg);
            } else {
                responseStr = executeFunction(msg);
            }
        }

        return responseStr;
    }
}
