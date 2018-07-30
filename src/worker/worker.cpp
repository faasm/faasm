#include <infra/infra.h>
#include "worker.h"

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
            int returnCode = wavm::executeModule(filePath.c_str());

            // Set function success
            std::cout << "Finished call:  " << call.user() << " - " << call.function() << "\n";
            std::cout << "Return code: " << returnCode << "\n";

            redis.setFunctionResult(call, true);
        }
    }
}