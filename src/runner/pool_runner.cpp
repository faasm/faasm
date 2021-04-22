#include <faabric/util/logging.h>

#include <faaslet/FaasletPool.h>

#include <faabric/endpoint/FaabricEndpoint.h>
#include <faabric/executor/FaabricMain.h>

#include <faabric/util/timing.h>

using namespace faabric::executor;
using namespace faaslet;

int main()
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();

    // Start the worker pool
    logger->info("Starting faaslet pool in the background");
    FaasletPool p(5);
    FaabricMain w(p);
    w.startBackground();

    // Start endpoint (will also have multiple threads)
    logger->info("Starting endpoint");
    faabric::endpoint::FaabricEndpoint endpoint;
    endpoint.start();
    PROF_BEGIN

    logger->info("Shutting down endpoint");
    PROF_SUMMARY
    w.shutdown();

    return EXIT_SUCCESS;
}
