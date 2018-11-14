#include "worker.h"

#include <infra/infra.h>
#include <wasm/wasm.h>
#include <spdlog/spdlog.h>

#include <thread>

namespace worker {

    Worker::Worker() = default;

    void Worker::start() {
        util::initLogging();

        while(true) {
            infra::Redis *redis = infra::Redis::getThreadConnection();

            bool wasFuncExecd = execNextFunction();

            // If we've timed out, reconnect
            if(!wasFuncExecd) {
                redis->refresh();
            }
        }
    }
}

int main() {
    worker::Worker worker;

    worker.start();
}
