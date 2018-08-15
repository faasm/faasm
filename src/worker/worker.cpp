#include "worker.h"
#include <infra/infra.h>

#include "Programs/CLI.h"
#include <IR/Module.h>
#include <Runtime/Linker.h>
#include <Inline/HashMap.h>
#include <Emscripten/Emscripten.h>

using namespace IR;
using namespace Runtime;

namespace worker {

    Worker::Worker() {

    }

    void Worker::start() {
        // Arbitrary loop to stop linting complaining
        for (int i = 0; i < 1000; i++) {
            std::cout << "Worker waiting..." << std::endl;

            // Get next call
            message::FunctionCall call = redis.nextFunctionCall();

            WasmModule module;
            module.execute(call);

            // Check for chained calls
            if(module.getChainCount() > 0) {
                for(size_t c = 0; c < module.getChainCount(); c++) {
                    std::string funcName = module.getChainName(c);
                    std::vector<U8> chainData = module.getChainData(c);

                    // Build chained function object
                    message::FunctionCall chainCall;
                    chainCall.set_user(call.user());
                    chainCall.set_function(funcName);
                    chainCall.set_outputdata(chainData.data(), chainData.size());

                    // Call the chained function
                    redis.callFunction(chainCall);
                }
            }

            module.clean();

            // Set function success
            std::cout << "Finished call:  " << call.user() << " - " << call.function() << std::endl;

            redis.setFunctionResult(call, true);
        }
    }
}

int main() {
    worker::Worker worker;

    worker.start();
}
