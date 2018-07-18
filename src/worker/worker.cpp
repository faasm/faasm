#include <redis/redis.h>
#include "worker/worker.h"

namespace worker {
    Worker::Worker() {

    }

    void Worker::start() {
        redis.connect();

        for(int i=0; i<50; i++) {
            std::cout << "Worker waiting...\n";

            message::FunctionCall call = redis.nextFunctionCall();

            std::cout << "Received call:  " << call.user() << " - " << call.function() << "\n";
        }

    }
}