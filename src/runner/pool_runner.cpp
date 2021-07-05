#include <faaslet/Faaslet.h>

#include <faabric/endpoint/FaabricEndpoint.h>
#include <faabric/runner/FaabricMain.h>
#include <faabric/transport/context.h>
#include <faabric/util/logging.h>

int main()
{
    faabric::transport::initGlobalMessageContext();
    faabric::util::initLogging();

    // WARNING: All 0MQ-related operations must take place in a self-contined
    // scope to ensure all sockets are destructed before closing the context.
    {
        auto fac = std::make_shared<faaslet::FaasletFactory>();
        faabric::runner::FaabricMain m(fac);
        m.startBackground();

        // Start endpoint (will also have multiple threads)
        SPDLOG_INFO("Starting endpoint");
        faabric::endpoint::FaabricEndpoint endpoint;
        endpoint.start();

        SPDLOG_INFO("Shutting down");
        m.shutdown();

        faabric::transport::closeGlobalMessageContext();
    }

    return 0;
}
