#include "KnativeEndpoint.h"

#include <util/logging.h>
#include <worker/WorkerMain.h>

using namespace knative;

int main() {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    // Start the worker
    worker::WorkerMain w;
    w.startBackground();

    // Start endpoint (listening for requests)
    logger->info("Starting knative endpoint");
    KnativeEndpoint endpoint;
    endpoint.start();

    logger->info("Shutting down knative endpoint");
    w.shutdown();

    return 0;
}
