#include "worker.h"

#include <wasm/wasm.h>

namespace worker {
    // TODO - how to choose an appropriate value for this?
    static int WORKER_THREADS = 10;

    static thread_local infra::Redis redis;

    static util::TokenPool tokenPool(WORKER_THREADS);

    void execNextFunction() {
        // Try to get an available slot
        int threadIdx = tokenPool.getToken();

        // Get next call (blocking)
        std::cout << "Worker waiting on slot " << threadIdx << "..." << std::endl;
        message::FunctionCall call = redis.nextFunctionCall();

        // New thread to execute function
        std::thread funcThread(execFunction, threadIdx, std::move(call));

        // Execute
        funcThread.detach();
    }

    void finishCall(message::FunctionCall &call, const std::string &errorMessage) {
        std::cout << "Finished call:  " << call.user() << " - " << call.function() << std::endl;

        bool isSuccess = errorMessage.empty();

        if (!isSuccess) {
            call.set_outputdata(errorMessage);
        }

        redis.setFunctionResult(call, isSuccess);
    }

    /** Handles the execution of the function */
    void execFunction(int index, message::FunctionCall call) {
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

        std::cout << "Starting call:  " << call.user() << " - " << call.function() << std::endl;

        // Create and execute the module
        wasm::WasmModule module;
        try {
            module.execute(call);
        }
        catch (const std::exception &e) {
            std::cout << "Error in wasm execution:\n" << e.what() << "\n" << std::endl;
            return finishCall(call, "Error in execution");
        }
        catch (...) {
            std::cout << "Unknown error in wasm execution" << std::endl;
            return finishCall(call, "Error in execution");
        }

        // Revert to original network namespace to allow communication
        ns.removeCurrentThread();

        // Release the token
        std::cout << "Worker releasing slot " << index << std::endl;
        tokenPool.releaseToken(index);

        // Dispatch any chained calls
        for (auto chainedCall : module.chainedCalls) {
            // Check if call is valid
            if (!infra::isValidFunction(chainedCall)) {
                std::string errorMessage = "Invalid chained function call: ";
                errorMessage.append(call.user());
                errorMessage.append(" - ");
                errorMessage.append(call.function());

                return finishCall(call, errorMessage);
            }

            redis.callFunction(chainedCall);
        }

        finishCall(call, "");

        // TODO running this clean() seems to kill the WAVM execution in all threads. Memory leak if not called?
        //module.clean();
    }
}
