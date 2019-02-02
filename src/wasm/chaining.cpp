#include "wasm.h"

#include <scheduler/Scheduler.h>

namespace wasm {
    CallChain::CallChain(const message::Message &msg) :
            originalCall(msg) {
    }

    void CallChain::addCall(std::string user, std::string functionName, const std::vector<uint8_t> &inputData) {
        message::Message msg;
        msg.set_user(user);
        msg.set_function(functionName);
        msg.set_inputdata(inputData.data(), inputData.size());

        calls.push_back(msg);
    }

    std::string CallChain::execute() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Dispatch any chained calls
        for (auto chainedCall : calls) {
            // Check if call is valid
            const std::string chainedStr = util::funcToString(chainedCall);
            if (!util::isValidFunction(chainedCall)) {
                std::string errorMessage = "Invalid chained function call: " + chainedStr;

                return errorMessage;
            }

            const std::string origStr = util::funcToString(originalCall);

            // TODO: Avoid this approach, add some concept of delay in the scheduler?
            // Wait a bit before chaining the same function
            if (chainedCall.function() == originalCall.function()) {
                logger->debug("Delaying chained call {} -> {}", origStr, chainedStr);
                uint microseconds = (uint) 500 * 1000; // 500ms
                usleep(microseconds);
            }

            logger->debug("Chaining {} -> {}", origStr, chainedStr);
            scheduler::Scheduler &sch = scheduler::getScheduler();
            sch.callFunction(chainedCall);
        }

        std::string empty;
        return empty;
    }
}
