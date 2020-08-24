#include "FaasletEndpoint.h"

#include <faabric/util/logging.h>
#include <faaslet/FaasmMain.h>

using namespace faaslet;

int main() {
    faabric::utilinitLogging();
    const std::shared_ptr<spdlog::logger> &logger = faabric::utilgetLogger();

    // Start the worker pool
    logger->info("Starting faaslet pool in the background");
    faaslet::FaasmMain w;
    w.startBackground();

    // Start endpoint (will also have multiple threads)
    logger->info("Starting endpoint");
    FaasletEndpoint endpoint;
    endpoint.start();

    logger->info("Shutting down endpoint");
    w.shutdown();

    return EXIT_SUCCESS;
}
