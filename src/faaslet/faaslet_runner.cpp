#include "FaasletEndpoint.h"

#include <util/logging.h>
#include <faaslet/FaasmMain.h>

using namespace faaslet;

int main() {
    util::initLogging();
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    // Start the worker pool
    logger->info("Starting worker pool in the background");
    faaslet::FaasmMain w;
    w.startBackground();

    // Start endpoint (will also have multiple threads)
    logger->info("Starting knative endpoint");
    FaasletEndpoint endpoint;
    endpoint.start();

    logger->info("Shutting down knative endpoint");
    w.shutdown();

    return EXIT_SUCCESS;
}
