#include <conf/FaasmConfig.h>
#include <faaslet/Faaslet.h>

#include <faabric/endpoint/FaabricEndpoint.h>
#include <faabric/runner/FaabricMain.h>
#include <faabric/scheduler/ExecutorFactory.h>
#include <faabric/util/logging.h>

using namespace faabric::scheduler;

int main()
{
    faabric::util::initLogging();

    SPDLOG_INFO("Starting distributed test server on worker");

    faabric::util::SystemConfig& faabricConf = faabric::util::getSystemConfig();
    conf::FaasmConfig& faasmConf = conf::getFaasmConfig();

    SPDLOG_INFO("--- Faabric config ---");
    faabricConf.print();

    SPDLOG_INFO("--- Faasm config ---");
    faasmConf.print();

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
