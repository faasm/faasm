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

#define ITERATIONS 50
#define CUBE_SIZE 20
#define REGIONS 11
#define BALANCE 1
#define COST 1

int doLuleshRun(int argc, char* argv[])
{
    faabric::util::setUpCrashHandler();
    faabric::util::initLogging();

    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    conf::FaasmConfig& faasmConf = conf::getFaasmConfig();

    faasmConf.chainedCallTimeout = 480000;
    faasmConf.s3Host = "localhost";

    conf.boundTimeout = 480000;
    conf.globalMessageTimeout = 480000;
//    conf.noSingleHostOptimisations = 1;

    int nThreads = faabric::util::getUsableCores();
    if (argc >= 2) {
        nThreads = std::stoi(argv[1]);
    }

    // Pass threads as input data
    std::string inputData = std::to_string(nThreads);

    // Commandline arguments
    std::string cmdlineArgs = fmt::format("-i {} -s {} -r {} -c {} -b {}",
                                          ITERATIONS,
                                          CUBE_SIZE,
                                          REGIONS,
                                          BALANCE,
                                          COST);
    auto req = faabric::util::batchExecFactory("lulesh", "func", 1);
    auto& msg = req->mutable_messages()->at(0);
    msg.set_cmdline(cmdlineArgs);

    // Clear out redis
    faabric::redis::Redis& redis = faabric::redis::Redis::getQueue();
    redis.flushAll();

    // Set up the system
    auto fac = std::make_shared<faaslet::FaasletFactory>();
    faabric::runner::FaabricMain m(fac);
    m.startRunner();

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

    m.shutdown();

    return 0;
}

int main(int argc, char* argv[])
{
    storage::initFaasmS3();
    faabric::transport::initGlobalMessageContext();

    int result = doLuleshRun(argc, argv);

    faabric::transport::closeGlobalMessageContext();
    storage::shutdownFaasmS3();

    return result;
}
