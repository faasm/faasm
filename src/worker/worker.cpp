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
        redis.connect();

        for (int i = 0; i < 50; i++) {
            std::cout << "Worker waiting...\n";

            // Get next call (blocking)
            message::FunctionCall call = redis.nextFunctionCall();

            WasmModule module;
            module.execute(call);

            // Set function success
            std::cout << "Finished call:  " << call.user() << " - " << call.function() << "\n";

            redis.setFunctionResult(call, true);
        }
    }
}
