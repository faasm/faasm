#include "worker.h"
#include <infra/infra.h>

#include <unistd.h>

#include "Programs/CLI.h"
#include <IR/Module.h>
#include <Runtime/Linker.h>
#include <Inline/HashMap.h>
#include <Emscripten/Emscripten.h>

using namespace IR;
using namespace Runtime;

namespace worker {

    static thread_local infra::Redis redis;

    Worker::Worker() {

    }

    void Worker::start() {
        // Arbitrary loop to stop linting complaining
        for (int i = 0; i < 1000; i++) {
            // Get next call (blocking)
            std::cout << "Worker waiting..." << std::endl;
            message::FunctionCall call = redis.nextFunctionCall();

            // Get cgroup for worker processes
            CGroup mainGroup("faasm");

            // Turn on CPU limiting
            mainGroup.limitCpu();

            // Create child process
            pid_t child = fork();
            if(child == 0) {
                // Add this child to the cgroup
                mainGroup.addCurrentPid();

                // Create and execute the module
                WasmModule module;
                module.execute(call);

                // Dispatch chained calls
                for(auto chainedCall : module.chainedCalls) {
                    redis.callFunction(chainedCall);
                }

                module.clean();

                // Set function success
                std::cout << "Finished call:  " << call.user() << " - " << call.function() << std::endl;

                redis.setFunctionResult(call, true);

                break;
            }
        }
    }
}

int main() {
    worker::Worker worker;

    worker.start();
}
