#include "worker.h"

#include <wasm/wasm.h>

#include <thread>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>


namespace worker {
    static std::string TOP_LEVEL_CGROUP = "faasm";
    static std::string TOP_LEVEL_NETNS = "faasm";
    static int WORKER_THREADS = 10;

    static util::TokenPool tokenPool(WORKER_THREADS);

    // TODO - unfortunately redis client can't be shared between threads
    static thread_local infra::Redis redis;

    /** Handles the execution of the function */
    void execFunction(int index, message::FunctionCall call) {
        auto tid = (pid_t) syscall(SYS_gettid);

        // Add this thread to the cgroup
        CGroup cgroup(TOP_LEVEL_CGROUP);
        cgroup.addThread(tid);

        // Set up network namespace
        std::string netNsName = TOP_LEVEL_NETNS + std::to_string(index);
        NetworkNamespace ns(netNsName);
        ns.addThread(tid);

        std::cout << "Starting call:  " << call.user() << " - " << call.function() << std::endl;

        // Create and execute the module
        wasm::WasmModule module;
        module.execute(call);

        // Release the token
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

    Worker::Worker() = default;

    void Worker::start() {
        // Create main cgroup with CPU limiting
        CGroup cgroup(TOP_LEVEL_CGROUP);
        cgroup.limitCpu();

        // Arbitrary loop to stop linting complaining
        for (int i = 0; i < 1000; i++) {
            // Try to get an available slot
            int threadIdx = tokenPool.getToken();

            // Get next call (blocking)
            std::cout << "Worker waiting..." << std::endl;
            message::FunctionCall call = redis.nextFunctionCall();

            // New thread to execute function
            std::thread funcThread(execFunction, threadIdx, std::move(call));

            // Execute
            funcThread.detach();
        }
    }
}

int main() {
    worker::Worker worker;

    worker.start();
}
