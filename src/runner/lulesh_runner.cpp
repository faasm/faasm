#include <filesystem>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <faabric/proto/faabric.pb.h>
#include <faabric/runner/FaabricMain.h>
#include <faabric/scheduler/Scheduler.h>
#include <faabric/transport/context.h>
#include <faabric/util/config.h>
#include <faabric/util/crash.h>
#include <faabric/util/environment.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <faabric/util/logging.h>
#include <faabric/util/timing.h>

#include <conf/FaasmConfig.h>
#include <faaslet/Faaslet.h>
#include <storage/S3Wrapper.h>
#include <wamr/WAMRWasmModule.h>
#include <wasm/WasmModule.h>
#include <wavm/WAVMWasmModule.h>

using namespace faabric::util;

#define LULESH_THREADS 8
#define HOST_THREADS 18

int doLuleshRun(int argc, char* argv[])
{
    setUpCrashHandler();
    initLogging();

    SystemConfig& conf = getSystemConfig();
    int nThreads = LULESH_THREADS;
    conf.overrideCpuCount = HOST_THREADS;

    std::string cmdlineArgs = "-i 2 -s 10 -r 11 -c 1 -b 1";
    std::string inputData = std::to_string(nThreads);

    auto req = batchExecFactory("lulesh", "func", 1);
    auto& msg = req->mutable_messages()->at(0);
    msg.set_cmdline(cmdlineArgs);
    msg.set_inputdata(inputData);

    // Clear out redis
    faabric::redis::Redis& redis = faabric::redis::Redis::getQueue();
    redis.flushAll();

    // Set up the system
    auto fac = std::make_shared<faaslet::FaasletFactory>();
    faabric::runner::FaabricMain m(fac);
    m.startRunner();

    // Timing
    TimePoint tp = startTimer();

    // Submit the invocation
    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    sch.callFunctions(req);

    // Await the result
    const faabric::Message& result =
      sch.getFunctionResult(msg.id(), conf.globalMessageTimeout);
    if (result.returnvalue() != 0) {
        SPDLOG_ERROR("Execution failed: {}", result.outputdata());
        throw std::runtime_error("Executing function failed");
    }

    long timeTaken = getTimeDiffMillis(tp);
    SPDLOG_INFO("TIME: {}", timeTaken);

    m.shutdown();

    return 0;
}

int main(int argc, char* argv[])
{
    SystemConfig& conf = getSystemConfig();
    conf::FaasmConfig& faasmConf = conf::getFaasmConfig();

    faasmConf.chainedCallTimeout = 480000;
    // faasmConf.s3Host = "localhost";
    conf.boundTimeout = 480000;
    conf.globalMessageTimeout = 480000;

    storage::initFaasmS3();
    faabric::transport::initGlobalMessageContext();

    PROF_BEGIN

    int result = doLuleshRun(argc, argv);

    PROF_SUMMARY

    faabric::transport::closeGlobalMessageContext();
    storage::shutdownFaasmS3();

    return result;
}
