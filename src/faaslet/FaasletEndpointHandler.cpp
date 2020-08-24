#include "FaasletEndpointHandler.h"

#include <faabric/util/logging.h>
#include <faabric/util/timing.h>
#include <faabric/util/json.h>
#include <faabric/scheduler/Scheduler.h>
#include <faabric/redis/Redis.h>

namespace faaslet {
    void FaasletEndpointHandler::onTimeout(const Pistache::Http::Request &request, Pistache::Http::ResponseWriter writer) {
        writer.send(Pistache::Http::Code::No_Content);
    }

    void FaasletEndpointHandler::onRequest(const Pistache::Http::Request &request, Pistache::Http::ResponseWriter response) {
        const std::shared_ptr<spdlog::logger> &logger = faabric::utilgetLogger();
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
        faabric::utilSystemConfig &conf = faabric::utilgetSystemConfig();
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
            faabric::Message msg = faabric::utiljsonToMessage(requestStr);
            scheduler::Scheduler &sched = scheduler::getScheduler();

            if (msg.isstatusrequest()) {
                responseStr = sched.getMessageStatus(msg.id());

            } else if(msg.isexecgraphrequest()) {
                scheduler::ExecGraph execGraph = sched.getFunctionExecGraph(msg.id());
                responseStr = scheduler::execGraphToJson(execGraph);

            } else if (msg.isflushrequest()) {
                const std::shared_ptr<spdlog::logger> &logger = faabric::utilgetLogger();
                logger->debug("Broadcasting flush request");

                sched.broadcastFlush(msg);
            } else {
                responseStr = executeFunction(msg);
            }
        }

        return responseStr;
    }

    std::string FaasletEndpointHandler::executeFunction(faabric::Message &msg) {
        const std::shared_ptr<spdlog::logger> &logger = faabric::utilgetLogger();
        faabric::utilSystemConfig &conf = faabric::utilgetSystemConfig();

        if (msg.user().empty()) {
            return "Empty user";
        } else if (msg.function().empty()) {
            return "Empty function";
        }

        faabric::utilsetMessageId(msg);

        auto tid = (pid_t) syscall(SYS_gettid);

        const std::string funcStr = faabric::utilfuncToString(msg, true);
        logger->debug("Worker HTTP thread {} scheduling {}", tid, funcStr);

        // Schedule it
        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.callFunction(msg);

        // Await result on global bus (may have been executed on a different worker)
        if (msg.isasync()) {
            return faabric::utilbuildAsyncResponse(msg);
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
