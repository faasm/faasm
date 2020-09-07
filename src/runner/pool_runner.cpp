#include <faabric/util/logging.h>

#include <faaslet/FaasletPool.h>

#include <faabric/executor/FaabricMain.h>
#include <faabric/endpoint/FaabricEndpoint.h>

using namespace faabric::executor;
using namespace faaslet;

int main() {
    faabric::util::initLogging();
    const std::shared_ptr<spdlog::logger> &logger = faabric::util::getLogger();

    // Start the worker pool
    logger->info("Starting faaslet pool in the background");
    FaasletPool p(5);
    FaabricMain w(p);
    w.startBackground();

    // Start endpoint (will also have multiple threads)
    logger->info("Starting endpoint");
    faabric::endpoint::FaabricEndpoint endpoint;
    endpoint.start();

    logger->info("Shutting down endpoint");
    w.shutdown();

    return EXIT_SUCCESS;
}
