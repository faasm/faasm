#include <cstdlib>

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

#define KERNEL_USER "omp"

#define HOST_THREADS 10

int doKernelRun(const std::string& kernelName,
                const std::vector<int> cmdlineArgs,
                int nThreads)
{
    SPDLOG_INFO("--- KERNEL RUN {} {} ---", kernelName, nThreads);

    setUpCrashHandler();
    initLogging();

    std::string cmdlineString = std::to_string(nThreads);
    for (auto c : cmdlineArgs) {
        cmdlineString += " " + std::to_string(c);
    }

    SystemConfig& conf = getSystemConfig();
    conf.overrideCpuCount = HOST_THREADS;

    auto req = batchExecFactory(KERNEL_USER, kernelName, 1);
    auto& msg = req->mutable_messages()->at(0);
    msg.set_cmdline(cmdlineString);

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

int doKernelsRun(int argc, char* argv[])
{
    std::vector<std::string> kernels = { "dgemm", "nstream", "sparse", "p2p" };
    if (argc > 1) {
        kernels = { argv[1] };
    }

    int nThreads = 4;

    std::map<std::string, std::vector<int>> cmdline = {
        { "dgemm", { 1, 2048, 32 } },
        { "nstream", { 10, 50000000, 32 } },
        { "sparse", { 10, 10, 12 } },
        { "p2p", { 10, 10000, 10000 } },
    };

    for (auto& kernel : kernels) {
        std::vector<int> cmdlineArgs = cmdline[kernel];
        doKernelRun(kernel, cmdlineArgs, nThreads);
    }

    return 0;
}

int main(int argc, char* argv[])
{
    //::setenv(
    //  "FAASM_LOCAL_DIR", "/home/shillaker/code/faasm/dev/faasm-local", 1);

    SystemConfig& conf = getSystemConfig();
    conf.boundTimeout = 480000;
    conf.globalMessageTimeout = 480000;
    conf.noSingleHostOptimisations = 0;

    conf.dirtyTrackingMode = "segfault";
    conf.diffingMode = "bytewise";

    conf::FaasmConfig& faasmConf = conf::getFaasmConfig();
    faasmConf.chainedCallTimeout = 480000;
    //faasmConf.s3Host = "localhost";

    storage::initFaasmS3();
    faabric::transport::initGlobalMessageContext();

    PROF_BEGIN
    int result = doKernelsRun(argc, argv);
    PROF_SUMMARY

    faabric::transport::closeGlobalMessageContext();
    storage::shutdownFaasmS3();
    return result;
}
