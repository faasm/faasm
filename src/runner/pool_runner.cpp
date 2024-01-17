#include <conf/FaasmConfig.h>
#include <faabric/endpoint/FaabricEndpoint.h>
#include <faabric/runner/FaabricMain.h>
#include <faabric/util/logging.h>
#include <faaslet/Faaslet.h>
#include <storage/S3Wrapper.h>

int main()
{
    storage::initFaasmS3();

    faabric::util::initLogging();

    // Print the Faasm config
    conf::getFaasmConfig().print();

    auto fac = std::make_shared<faaslet::FaasletFactory>();
    faabric::runner::FaabricMain m(fac);
    m.startBackground();

    // Start endpoint (will also have multiple threads)
    SPDLOG_INFO("Starting endpoint");
    faabric::endpoint::FaabricEndpoint endpoint;
    endpoint.start(faabric::endpoint::EndpointMode::SIGNAL);

    SPDLOG_INFO("Shutting down");
    m.shutdown();

    storage::shutdownFaasmS3();
    return 0;
}
