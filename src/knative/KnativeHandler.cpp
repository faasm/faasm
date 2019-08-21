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

        const std::string responseStr = handleFunction(requestStr);

        PROF_END(knativeRoundTrip)
        response.send(Pistache::Http::Code::Ok, responseStr);
    }

    std::string KnativeHandler::handleFunction(const std::string &requestStr) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if (requestStr.empty()) {
            return "Empty request";
        }

        // Parse the message
        message::Message msg = util::jsonToMessage(requestStr);

        if (msg.user().empty()) {
            return "Empty user";
        } else if (msg.function().empty()) {
            return "Empty function";
        }

        util::setMessageId(msg);

        auto tid = (pid_t) syscall(SYS_gettid);

        const std::string funcStr = util::funcToString(msg, true);
        logger->debug("Knative thread {} scheduling {}", tid, funcStr);

        // Schedule it
        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.callFunction(msg);

        // Await result on global bus (may have been executed on a different worker)
        if (msg.isasync()) {
            return "Async request received";
        } else {
            logger->debug("Knative thread {} awaiting {}", tid, funcStr);

            try {
                scheduler::GlobalMessageBus &globalBus = scheduler::getGlobalMessageBus();
                const message::Message result = globalBus.getFunctionResult(msg.id(), conf.globalMessageTimeout);
                logger->debug("Knative thread {} result {}", tid, funcStr);

                return result.outputdata() + "\n";
            } catch (redis::RedisNoResponseException &ex) {
                return "No response from function\n";
            }
        }
    }
}
