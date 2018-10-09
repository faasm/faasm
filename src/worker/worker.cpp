#include "worker.h"

#include <wasm/wasm.h>

#include <thread>

namespace worker {

    Worker::Worker() = default;

    void Worker::start() {
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
