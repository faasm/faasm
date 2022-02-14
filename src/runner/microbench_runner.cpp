#include <string>

#include <conf/FaasmConfig.h>
#include <faaslet/Faaslet.h>
#include <runner/MicrobenchRunner.h>
#include <storage/S3Wrapper.h>

#include <faabric/runner/FaabricMain.h>
#include <faabric/scheduler/ExecutorFactory.h>
#include <faabric/scheduler/Scheduler.h>
#include <faabric/transport/context.h>
#include <faabric/util/config.h>
#include <faabric/util/logging.h>
#include <faabric/util/timing.h>

#include <boost/filesystem.hpp>

using namespace faabric::util;
using namespace runner;

int main(int argc, char* argv[])
{
    storage::initFaasmS3();
    faabric::transport::initGlobalMessageContext();

    initLogging();

    if (argc < 3) {
        SPDLOG_ERROR("Usage: microbench_runner <infile> <outfile> [--sgx]");
        return 1;
    }

    bool isSgx = false;
    if (argc == 4 && std::string(argv[3]) == "--sgx") {
        isSgx = true;
    }

    // Process input args
    std::string inFile = argv[1];
    std::string outFile = argv[2];

    // Set up config
    SystemConfig& conf = getSystemConfig();
    conf::FaasmConfig& faasmConf = conf::getFaasmConfig();

    conf.boundTimeout = 60000;
    conf.globalMessageTimeout = 60000;
    faasmConf.chainedCallTimeout = 60000;

    // Set executor factory
    std::shared_ptr<faaslet::FaasletFactory> fac =
      std::make_shared<faaslet::FaasletFactory>();
    faabric::scheduler::setExecutorFactory(fac);

    int returnValue = MicrobenchRunner::execute(inFile, outFile, isSgx);

    faabric::transport::closeGlobalMessageContext();
    storage::shutdownFaasmS3();
    return returnValue;
}
