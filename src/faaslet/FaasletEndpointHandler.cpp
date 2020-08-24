#include "FaasletEndpointHandler.h"

#include <util/logging.h>
#include <util/timing.h>
#include <util/json.h>
#include <scheduler/Scheduler.h>
#include <redis/Redis.h>

namespace faaslet {
    void FaasletEndpointHandler::onTimeout(const Pistache::Http::Request &request, Pistache::Http::ResponseWriter writer) {
        writer.send(Pistache::Http::Code::No_Content);
    }

    void FaasletEndpointHandler::onRequest(const Pistache::Http::Request &request, Pistache::Http::ResponseWriter response) {
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

    std::string FaasletEndpointHandler::handleFunction(const std::string &requestStr) {
        std::string responseStr;
        if (requestStr.empty()) {
            responseStr = "Empty request";
        } else {
            faabric::Message msg = util::jsonToMessage(requestStr);
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

    std::string FaasletEndpointHandler::executeFunction(faabric::Message &msg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        util::SystemConfig &conf = util::getSystemConfig();

        if (msg.user().empty()) {
            return "Empty user";
        } else if (msg.function().empty()) {
            return "Empty function";
        }

        util::setMessageId(msg);

        auto tid = (pid_t) syscall(SYS_gettid);

        const std::string funcStr = util::funcToString(msg, true);
        logger->debug("Worker HTTP thread {} scheduling {}", tid, funcStr);

        // Schedule it
        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.callFunction(msg);

        // Await result on global bus (may have been executed on a different worker)
        if (msg.isasync()) {
            return util::buildAsyncResponse(msg);
        } else {
            logger->debug("Worker thread {} awaiting {}", tid, funcStr);

            try {
                const faabric::Message result = sch.getFunctionResult(msg.id(), conf.globalMessageTimeout);
                logger->debug("Worker thread {} result {}", tid, funcStr);

                return result.outputdata() + "\n";
            } catch (redis::RedisNoResponseException &ex) {
                return "No response from function\n";
            }
        }
    }
}
