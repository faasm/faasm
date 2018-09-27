#include "worker.h"

#include <wasm/wasm.h>

#include <thread>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>


namespace worker {
    static int WORKER_THREADS = 10;

    static util::TokenPool tokenPool(WORKER_THREADS);

    // TODO - unfortunately redis client can't be shared between threads
    static thread_local infra::Redis redis;

    /** Handles the execution of the function */
    void execFunction(int index, message::FunctionCall call) {
        // Add this thread to the cgroup
        CGroup cgroup;
        auto tid = (pid_t) syscall(SYS_gettid);
        cgroup.addThread(tid, CG_CPU);

        // Set up network namespace
        NetworkNamespace ns(index);
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

    Worker::Worker() = default;

    void Worker::start() {
        // Create main cgroup with CPU limiting
        CGroup cgroup;
        cgroup.addController(CG_CPU);

        // Arbitrary loop to stop linting complaining
        for (int i = 0; i < 1000; i++) {
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
    }
}

int main() {
    worker::Worker worker;

    worker.start();
}
