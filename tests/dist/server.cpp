#include <faaslet/Faaslet.h>

#include <faabric/endpoint/FaabricEndpoint.h>
#include <faabric/runner/FaabricMain.h>
#include <faabric/scheduler/ExecutorFactory.h>
#include <faabric/transport/context.h>
#include <faabric/util/logging.h>

using namespace faabric::scheduler;

int main()
{
    faabric::transport::initGlobalMessageContext();
    faabric::util::initLogging();

    // WARNING: All 0MQ-related operations must take place in a self-contined
    // scope to ensure all sockets are destructed before closing the context.
    {
        SPDLOG_INFO("Starting distributed test server on worker");
        std::shared_ptr<ExecutorFactory> fac =
          std::make_shared<faaslet::FaasletFactory>();
        faabric::runner::FaabricMain m(fac);
        m.startBackground();

        SPDLOG_INFO("Starting HTTP endpoint on worker");
        faabric::endpoint::FaabricEndpoint endpoint;
        endpoint.start();

        SPDLOG_INFO("Shutting down");
        m.shutdown();
    }

    faabric::transport::closeGlobalMessageContext();

    return 0;
}
