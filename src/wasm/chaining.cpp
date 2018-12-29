#include "wasm.h"

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
            if (!infra::isValidFunction(chainedCall)) {
                std::string errorMessage = "Invalid chained function call: " + infra::funcToString(chainedCall);

                return errorMessage;
            }

            // TODO: Avoid this, add some concept of delay in the scheduler?
            // Wait a bit before chaining the same function
            if (chainedCall.function() == originalCall.function()) {
                logger->debug("Delaying chained call {} -> {}", infra::funcToString(originalCall),
                              infra::funcToString(chainedCall));
                uint microseconds = (uint) 500 * 1000; // 500ms
                usleep(microseconds);
            }

            logger->debug("Chaining {} -> {}", infra::funcToString(originalCall), infra::funcToString(chainedCall));
            infra::Scheduler::callFunction(chainedCall);
        }

        std::string empty;
        return empty;
    }
}
