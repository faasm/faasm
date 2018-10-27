#include "worker.h"

#include <wasm/wasm.h>

namespace worker {
    // TODO - how to choose an appropriate value for this?
    static int WORKER_THREADS = 10;

    static util::TokenPool tokenPool(WORKER_THREADS);

    void execNextFunction() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Try to get an available slot
        int threadIdx = tokenPool.getToken();

        // Get next call (blocking)
        logger->debug("Worker waiting on slot {}", threadIdx);
        infra::Redis *redis = infra::Redis::getThreadConnection();
        message::FunctionCall call = redis->nextFunctionCall();

        // New thread to execute function
        std::thread funcThread(execFunction, threadIdx, std::move(call));

        // Execute
        funcThread.detach();
    }

    void finishCall(message::FunctionCall &call, const std::string &errorMessage) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->info("Finished ({}/{})", call.user(), call.function());

        bool isSuccess = errorMessage.empty();

        if (!isSuccess) {
            call.set_outputdata(errorMessage);
        }

        infra::Redis *redis = infra::Redis::getThreadConnection();
        redis->setFunctionResult(call, isSuccess);
    }

    /** Handles the execution of the function */
    void execFunction(int index, message::FunctionCall call) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if (!infra::isValidFunction(call)) {
            std::string errorMessage = call.user();
            errorMessage.append(" - ");
            errorMessage.append(call.function());
            errorMessage.append(" is not a valid function");

            return finishCall(call, errorMessage);
        }

        // Note, we index cgroups from 1
        int cg_index = index + 1;

        // Add this thread to the cgroup
        CGroup cgroup(BASE_CGROUP_NAME);
        cgroup.addCurrentThread();

        // Set up network namespace
        std::string netnsName = BASE_NETNS_NAME + std::to_string(cg_index);
        NetworkNamespace ns(netnsName);
        ns.addCurrentThread();

        logger->info("Starting ({}/{})", call.user(), call.function());

        // Create and execute the module
        wasm::WasmModule module;
        try {
            module.execute(call);
        }
        catch (const std::exception &e) {
            std::string errorMessage = "Error: " + std::string(e.what());
            logger->error(errorMessage);

            return finishCall(call, errorMessage);
        }
        catch (...) {
            logger->error("Unknown error in wasm execution");

            return finishCall(call, "Error in execution");
        }

        // Revert to original network namespace to allow communication
        ns.removeCurrentThread();

        // Release the token
        logger->debug("Worker releasing slot {}", index);

        tokenPool.releaseToken(index);

        // Dispatch any chained calls
        infra::Redis *redis = infra::Redis::getThreadConnection();
        for (auto chainedCall : module.chainedCalls) {
            // Check if call is valid
            if (!infra::isValidFunction(chainedCall)) {
                std::string errorMessage = "Invalid chained function call: ";
                errorMessage.append(call.user());
                errorMessage.append(" - ");
                errorMessage.append(call.function());

                return finishCall(call, errorMessage);
            }

            logger->debug("Chaining ({}/{})", chainedCall.user(), chainedCall.function());
            redis->callFunction(chainedCall);
        }

        finishCall(call, "");
    }
}
