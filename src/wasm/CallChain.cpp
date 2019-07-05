#include "CallChain.h"

#include <scheduler/Scheduler.h>

namespace wasm {
    CallChain::CallChain(const message::Message &msg) :
            originalCall(msg) {
    }

    void CallChain::addCall(std::string user, std::string functionName, const std::vector<uint8_t> &inputData) {
        this->addCallThis(user, functionName, 0, inputData);
    }

    void CallChain::addCallThis(std::string user, std::string functionName, int idx, const std::vector<uint8_t> &inputData) {
        message::Message msg;
        msg.set_user(user);
        msg.set_function(functionName);
        msg.set_inputdata(inputData.data(), inputData.size());
        msg.set_idx(idx)

        calls.push_back(msg);
    }

    std::string CallChain::execute() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        scheduler::Scheduler &sch = scheduler::getScheduler();

        // Dispatch any chained calls
        for (auto chainedCall : calls) {
            const std::string chainedStr = util::funcToString(chainedCall);
            const std::string origStr = util::funcToString(originalCall);

            // TODO: Avoid this approach to recursive calls, add some option of delay in the scheduler?
            // Wait a bit before making a recursive call
            if (chainedCall.function() == originalCall.function()) {
                logger->debug("Delaying chained call {} -> {}", origStr, chainedStr);
                uint microseconds = (uint) 500 * 1000; // 500ms
                usleep(microseconds);
            }

            logger->debug("Chaining {} -> {}", origStr, chainedStr);
            util::addIdToMessage(chainedCall);
            sch.callFunction(chainedCall);
        }

        std::string empty;
        return empty;
    }
}
