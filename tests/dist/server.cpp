#include <conf/FaasmConfig.h>
#include <faaslet/Faaslet.h>

#include <faabric/endpoint/FaabricEndpoint.h>
#include <faabric/runner/FaabricMain.h>
#include <faabric/scheduler/ExecutorFactory.h>
#include <faabric/util/config.h>
#include <faabric/util/logging.h>

using namespace faabric::scheduler;

int main()
{
    faabric::util::initLogging();
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    conf.print();

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

    return EXIT_SUCCESS;
}
