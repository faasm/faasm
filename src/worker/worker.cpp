#include "worker.h"

#include <infra/infra.h>
#include <IR/Module.h>

namespace worker {
    Worker::Worker() {

    }

    void Worker::start() {
        redis.connect();

        for(int i=0; i<50; i++) {
            std::cout << "Worker waiting...\n";

            // Get next call (blocking)
            message::FunctionCall call = redis.nextFunctionCall();

            std::cout << "Received call:  " << call.user() << " - " << call.function() << "\n";

            std::string filePath = infra::getFunctionFile(call);

            IR::Module module;

            // Set function success
            std::cout << "Finished call:  " << call.user() << " - " << call.function() << "\n";

            redis.setFunctionResult(call, true);
        }
    }
}