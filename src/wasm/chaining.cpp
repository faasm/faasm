#include "wasm.h"

namespace wasm {
    CallChain::CallChain(const message::Message &call) :
            originalCall(call) {
    }

    void CallChain::addCall(std::string user, std::string functionName, const std::vector<uint8_t> &inputData) {
        message::Message call;
        call.set_user(user);
        call.set_function(functionName);
        call.set_inputdata(inputData.data(), inputData.size());

        calls.push_back(call);
    }

    std::string CallChain::execute() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Dispatch any chained calls
        infra::Redis *redis = infra::Redis::getThreadConnection();
        for (auto chainedCall : calls) {
            // Check if call is valid
            if (!infra::isValidFunction(chainedCall)) {
                std::string errorMessage = "Invalid chained function call: " + infra::funcToString(chainedCall);

                return errorMessage;
            }

            logger->debug("Chaining {} -> {}", infra::funcToString(originalCall), infra::funcToString(chainedCall));
            redis->callFunction(chainedCall);
        }

        std::string empty;
        return empty;
    }
}
