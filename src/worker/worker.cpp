#include "worker.h"
#include <infra/infra.h>

#include <thread>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include "Programs/CLI.h"
#include <IR/Module.h>
#include <Runtime/Linker.h>
#include <Inline/HashMap.h>
#include <Emscripten/Emscripten.h>

using namespace IR;
using namespace Runtime;

namespace worker {

    // TODO - unfortunately redis client can't be shared between threads
    static thread_local infra::Redis redis;

    /** Handles the execution of the function */
    void execFunction(message::FunctionCall call, std::shared_ptr<CGroup> cgroup) {
        // Add this thread to the cgroup
        auto tid = (pid_t) syscall(SYS_gettid);
        cgroup->addThread(tid);

        // Create and execute the module
        WasmModule module;
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
        cgroup = std::make_shared<CGroup>("faasm");
        cgroup->limitCpu();

        // Arbitrary loop to stop linting complaining
        for (int i = 0; i < 1000; i++) {
            // Get next call (blocking)
            std::cout << "Worker waiting..." << std::endl;
            message::FunctionCall call = redis.nextFunctionCall();

            // New thread to execute function
            std::thread funcThread(execFunction, std::move(call), cgroup);

            // Execute
            funcThread.detach();
        }
    }
}

int main() {
    worker::Worker worker;

    worker.start();
}
