#include "worker.h"

#include <wasm/wasm.h>

namespace worker {
    // TODO - unfortunately redis client can't be shared between threads
    static thread_local infra::Redis redis;
    static int WORKER_THREADS = 10;

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

    /** Handles the execution of the function */
    void execFunction(int index, message::FunctionCall call) {
        // Note, we index cgroups from 1
        int cg_index = index + 1;

        // Add this thread to the cgroup
        std::string cgName = BASE_CGROUP_NAME + std::to_string(cg_index);
        CGroup cgroup(cgName);
        cgroup.addCurrentThread();

        // Set up network namespace
        std::string netnsName = BASE_NETNS_NAME + std::to_string(cg_index);
        NetworkNamespace ns(netnsName);
        ns.addCurrentThread();

        std::cout << "Starting call:  " << call.user() << " - " << call.function() << std::endl;

        // Create and execute the module
        wasm::WasmModule module;
        module.execute(call);

        // Release the token
        std::cout << "Worker releasing slot " << index << std::endl;
        tokenPool.releaseToken(index);

        // Dispatch any chained calls
        for (auto chainedCall : module.chainedCalls) {
            redis.callFunction(chainedCall);
        }

        // Set function success
        std::cout << "Finished call:  " << call.user() << " - " << call.function() << std::endl;
        redis.setFunctionResult(call, true);

        // TODO running this clean() seems to kill the WAVM execution in all threads. Memory leak if not called?
        //module.clean();
    }
}
