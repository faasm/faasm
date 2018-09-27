#include "worker.h"

#include <wasm/wasm.h>

#include <thread>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>


namespace worker {

    static std::string TOP_LEVEL_CGROUP = "faasm";
    static std::string TOP_LEVEL_NETNS = "faasm";

    static int N_WORKER_THREADS = 10;

    // TODO - unfortunately redis client can't be shared between threads
    static thread_local infra::Redis redis;

    /** Handles the execution of the function */
    void execFunction(int index, message::FunctionCall call) {
        // Add this thread to the cgroup
        auto tid = (pid_t) syscall(SYS_gettid);

        CGroup cgroup(TOP_LEVEL_CGROUP);
        cgroup.addThread(tid);

        std::string netNsName = TOP_LEVEL_NETNS + std::to_string(index);
        NetworkNamespace ns(netNsName);
        ns.addThread(tid);

        std::cout << "Starting call:  " << call.user() << " - " << call.function() << std::endl;

        // Create and execute the module
        wasm::WasmModule module;
        module.execute(call);

        // Dispatch any chained calls
        for (auto chainedCall : module.chainedCalls) {
            redis.callFunction(chainedCall);
        }

        // Set function success
        std::cout << "Finished call:  " << call.user() << " - " << call.function() << std::endl;
        redis.setFunctionResult(call, true);

        // TODO running this clean() seems to kill the WAVM execution in all threads
        //module.clean();
    }

    Worker::Worker() = default;

    void Worker::start() {
        // Create main cgroup with CPU limiting
        CGroup cgroup(TOP_LEVEL_CGROUP);
        cgroup.limitCpu();

        ThreadPool threadPool(N_WORKER_THREADS);

        // Arbitrary loop to stop linting complaining
        for (int i = 0; i < 1000; i++) {
            // Get next call (blocking)
            std::cout << "Worker waiting..." << std::endl;
            message::FunctionCall call = redis.nextFunctionCall();

            // See if we have a thread
            int threadIdx = -1;
            try {
                threadIdx = threadPool.getSetExecuting()  ;
            } catch (NoThreadsException e) {
                
            }


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
