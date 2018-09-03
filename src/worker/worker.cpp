#include "worker.h"
#include <infra/infra.h>

#include <thread>

#include "Programs/CLI.h"
#include <IR/Module.h>
#include <Runtime/Linker.h>
#include <Inline/HashMap.h>
#include <Emscripten/Emscripten.h>

using namespace IR;
using namespace Runtime;

namespace worker {

    static thread_local infra::Redis redis;

    void execFunction(std::shared_ptr<CGroup> cgroup, message::FunctionCall call) {
        // Add this thread to the cgroup
        cgroup->addCurrentThread();

        // Create and execute the module
        WasmModule module;
        module.execute(call);

        // TODO - unfortunately redis client can't be shared between threads
        // therefore need to create yet another connection
        infra::Redis loopRedis;

        // Dispatch any chained calls
        for (auto chainedCall : module.chainedCalls) {
            loopRedis.callFunction(chainedCall);
        }

        // Set function success
        std::cout << "Finished call:  " << call.user() << " - " << call.function() << std::endl;
        loopRedis.setFunctionResult(call, true);

        module.clean();
    }

    Worker::Worker() = default;

    void Worker::start() {
        // Create main CGroup with CPU limiting
        cgroup = std::make_shared<CGroup>("faasm");
        cgroup->limitCpu();

        // Arbitrary loop to stop linting complaining
        for (int i = 0; i < 1000; i++) {
            // Get next call (blocking)
            std::cout << "Worker waiting..." << std::endl;
            message::FunctionCall call = redis.nextFunctionCall();

            // Create new thread for this call
            std::thread funcThread(execFunction, cgroup, std::move(call));
            funcThread.detach();
        }
    }
}

int main() {
    worker::Worker worker;

    worker.start();
}
