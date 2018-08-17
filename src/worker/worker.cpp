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
        // Create main CGroup with CPU limiting
        CGroup mainGroup("faasm");
        mainGroup.limitCpu();

        // Arbitrary loop to stop linting complaining
        for (int i = 0; i < 1000; i++) {
            // Get next call (blocking)
            std::cout << "Worker waiting..." << std::endl;
            message::FunctionCall call = redis.nextFunctionCall();

            // Create child process
            pid_t child = fork();
            if(child == 0) {
                // Add this child to the cgroup
                mainGroup.addCurrentPid();

                // Create and execute the module
                WasmModule module;
                module.execute(call);

                // TODO - unfortunately redis client isn't possible to share between processes
                // therefore need to create yet another connection for the child process
                infra::Redis loopRedis;

                // Dispatch chained calls
                for(auto chainedCall : module.chainedCalls) {
                    loopRedis.callFunction(chainedCall);
                }

                // Set function success
                std::cout << "Finished call:  " << call.user() << " - " << call.function() << std::endl;
                loopRedis.setFunctionResult(call, true);

                module.clean();

                return;
            }
        }
    }
}

int main() {
    worker::Worker worker;

    worker.start();
}
