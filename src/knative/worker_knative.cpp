#include "KnativeEndpoint.h"

#include <util/logging.h>
#include <worker/WorkerMain.h>

using namespace knative;

int main() {

    util::initLogging();
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    // Start the worker pool
    logger->info("Starting worker pool in the background");
    worker::WorkerMain w;
    w.startBackground();

    // Start endpoint (will also have multiple threads)
    logger->info("Starting knative endpoint");
    KnativeEndpoint endpoint;
    endpoint.start();

    logger->info("Shutting down knative endpoint");
    w.shutdown();

    return EXIT_SUCCESS;
}
