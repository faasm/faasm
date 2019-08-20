#include "KnativeEndpoint.h"

#include <util/logging.h>
#include <worker/WorkerMain.h>
#include <knative/KnativeHandler.h>
#include <util/json.h>

using namespace knative;

int main() {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    worker::WorkerMain w;
    w.startBackground();

    std::vector<std::thread> threads(10);
    for (int i = 0; i < 10; i++) {
        threads.emplace_back(std::thread([&logger, i] {
            message::Message msg = util::messageFactory("demo", "echo");

            for (int j = 0; j < 10; j++) {
                logger->info("Thread {} request {}", i, j);
                KnativeHandler handler;
                handler.handleFunction(util::messageToJson(msg));
            }
        }));
    }

    for (auto &t : threads) {
        if(t.joinable()) {
            t.join();
        }
    }

    w.shutdown();

    return 0;
}