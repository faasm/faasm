#include <string>

#include <conf/FaasmConfig.h>
#include <faaslet/Faaslet.h>
#include <runner/MicrobenchRunner.h>
#include <storage/S3Wrapper.h>

#include <faabric/runner/FaabricMain.h>
#include <faabric/scheduler/ExecutorFactory.h>
#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/config.h>
#include <faabric/util/logging.h>
#include <faabric/util/timing.h>

#include <boost/filesystem.hpp>

using namespace faabric::util;
using namespace runner;

int main(int argc, char* argv[])
{
    storage::initFaasmS3();

    initLogging();

    if (argc < 3) {
        SPDLOG_ERROR("Usage: microbench_runner <infile> <outfile>");
        return 1;
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

    int returnValue = MicrobenchRunner::execute(inFile, outFile);
    storage::shutdownFaasmS3();
    return returnValue;
}
