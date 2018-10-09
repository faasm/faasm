#include "worker.h"

#include <wasm/wasm.h>

namespace worker {
    // TODO - how to choose an appropriate value for this?
    static int WORKER_THREADS = 10;

    static util::TokenPool tokenPool(WORKER_THREADS);

    void execNextFunction() {
        // Be careful not to share redis between threads
        static infra::Redis redis;

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

    /** Handles the execution of the function */
    void execFunction(int index, message::FunctionCall call) {
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
        std::string errorMessage = "";
        wasm::WasmModule module;
        try {
            module.execute(call);
        }
        catch(const std::exception &e) {
            std::cout << "Error in wasm execution:\n" << e.what() << "\n" << std::endl;
            errorMessage = "Error in execution";
        }
        catch(...) {
            std::cout << "Unknown error in wasm execution" << std::endl;
            errorMessage = "Error in execution";
        }

        // Revert to original network namespace and cgroup
        cgroup.removeCurrentThread();
        ns.removeCurrentThread();

        // Release the token
        std::cout << "Worker releasing slot " << index << std::endl;
        tokenPool.releaseToken(index);

        // Create redis client in this thread
        static infra::Redis redis;

        // Dispatch any chained calls
        for (auto chainedCall : module.chainedCalls) {
            // Check if call is valid
            if(!infra::isValidFunction(chainedCall)) {
                errorMessage = "Invalid chained function call: ";
                errorMessage.append(call.user());
                errorMessage.append(" - ");
                errorMessage.append(call.function());

                break;
            }

            redis.callFunction(chainedCall);
        }

        // Set function success
        std::cout << "Finished call:  " << call.user() << " - " << call.function() << std::endl;

        bool isSuccess = errorMessage.empty();
        redis.setFunctionResult(call, isSuccess);

        // TODO running this clean() seems to kill the WAVM execution in all threads. Memory leak if not called?
        //module.clean();
    }
}
