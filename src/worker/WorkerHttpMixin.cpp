#include "WorkerHttpMixin.h"

#include <util/logging.h>
#include <util/timing.h>
#include <scheduler/Scheduler.h>

namespace worker {
    std::string WorkerHttpMixin::getMessageStatus(message::Message &msg) {
        // Get message with no delay
        scheduler::GlobalMessageBus &globalBus = scheduler::getGlobalMessageBus();
        const message::Message result = globalBus.getFunctionResult(msg.id(), 0);

        if(result.type() == message::Message_MessageType_EMPTY) {
            return "RUNNING";
        } else if(result.success()) {
            return "SUCCESS: " + result.outputdata();
        } else {
            return "FAILED: " + result.outputdata();
        }
    }
    
    std::string WorkerHttpMixin::executeFunction(message::Message &msg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        util::SystemConfig &conf = util::getSystemConfig();

        if (msg.user().empty()) {
            return "Empty user";
        } else if (msg.function().empty()) {
            return "Empty function";
        }

        if (msg.ispython()) {
            util::convertMessageToPython(msg);
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
                scheduler::GlobalMessageBus &globalBus = scheduler::getGlobalMessageBus();
                const message::Message result = globalBus.getFunctionResult(msg.id(), conf.globalMessageTimeout);
                logger->debug("Worker thread {} result {}", tid, funcStr);

                return result.outputdata() + "\n";
            } catch (redis::RedisNoResponseException &ex) {
                return "No response from function\n";
            }
        }
    }
}
