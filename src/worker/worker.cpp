#include "worker.h"

#include <wasm/wasm.h>
#include <spdlog/spdlog.h>

#include <thread>

namespace worker {

    Worker::Worker() = default;

    void Worker::start() {
        util::initLogging();

        // Arbitrary loop to stop linting complaining
        for (int i = 0; i < INT32_MAX; i++) {
            execNextFunction();
        }
    }
}

int main() {
    worker::Worker worker;

    worker.start();
}
