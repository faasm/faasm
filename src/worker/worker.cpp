#include "worker.h"

#include <wasm/wasm.h>

#include <thread>

namespace worker {

    Worker::Worker() = default;

    void Worker::start() {
        // Set up cgroup for this host
        std::string hostname = util::getCurrentHostname();
        CGroup cgroup(hostname);
        cgroup.createIfNotExists();

        // Arbitrary loop to stop linting complaining
        for (int i = 0; i < 1000; i++) {
            execNextFunction();
        }
    }
}

int main() {
    worker::Worker worker;

    worker.start();
}
