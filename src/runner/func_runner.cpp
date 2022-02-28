#include "faabric/util/environment.h"
#include <conf/FaasmConfig.h>
#include <faaslet/Faaslet.h>
#include <storage/FileLoader.h>
#include <wasm/WasmModule.h>

#include <faabric/redis/Redis.h>
#include <faabric/runner/FaabricMain.h>
#include <faabric/scheduler/ExecutorFactory.h>
#include <faabric/transport/context.h>
#include <faabric/util/config.h>
#include <faabric/util/logging.h>
#include <faabric/util/timing.h>

int doRunner(int argc, char* argv[])
{
    faabric::util::initLogging();

    if (argc < 3) {
        SPDLOG_ERROR("Must provide user and function name");
        return 1;
    }

    // Set up the call
    std::string user = argv[1];
    std::string function = argv[2];

    std::string inputData;
    bool hasInput = false;
    if (argc == 4) {
        inputData = argv[3];
        hasInput = true;
    }

    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory(user, function, 1);
    faabric::Message& msg = req->mutable_messages()->at(0);

    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    conf::FaasmConfig& faasmConf = conf::getFaasmConfig();

    // Set timeout to ensure longer functions can finish
    conf.boundTimeout = 120000;
    conf.globalMessageTimeout = 120000;
    faasmConf.chainedCallTimeout = 120000;

    // Make sure we have enough space for chained calls
    int nThreads = std::min<int>(faabric::util::getUsableCores(), 10);
    faabric::HostResources res;
    res.set_slots(nThreads);
    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    sch.setThisHostResources(res);

    conf.defaultMpiWorldSize = 5;

    // Clear out redis
    faabric::redis::Redis& redis = faabric::redis::Redis::getQueue();
    redis.flushAll();

    if (user == "python") {
        msg.set_pythonuser(msg.user());
        msg.set_pythonfunction(msg.function());
        msg.set_ispython(true);

        msg.set_user(PYTHON_USER);
        msg.set_function(PYTHON_FUNC);

        SPDLOG_INFO("Running Python function {}/{}",
                    msg.pythonuser(),
                    msg.pythonfunction());
    } else {
        SPDLOG_INFO("Running function {}/{}", user, function);
    }

    if (hasInput) {
        std::string inputData = argv[3];
        msg.set_inputdata(inputData);

        SPDLOG_INFO("Adding input data: {}", inputData);
    }

    // Set up the system
    auto fac = std::make_shared<faaslet::FaasletFactory>();
    faabric::runner::FaabricMain m(fac);
    m.startRunner();

    // Submit the invocation
    PROF_START(FunctionExec)
    sch.callFunctions(req);

    // Await the result
    const faabric::Message& result =
      sch.getFunctionResult(msg.id(), conf.globalMessageTimeout);
    if (result.returnvalue() != 0) {
        SPDLOG_ERROR("Execution failed: {}", result.outputdata());
        throw std::runtime_error("Executing function failed");
    }

    PROF_END(FunctionExec)

    m.shutdown();

    return 0;
}

int main(int argc, char* argv[])
{
    conf::getFaasmConfig().s3Host = "localhost";
    storage::initFaasmS3();
    faabric::transport::initGlobalMessageContext();

    PROF_BEGIN

    // WARNING: All 0MQ-related operations must take place in a self-contined
    // scope to ensure all sockets are destructed before closing the context.
    int result = doRunner(argc, argv);

    PROF_SUMMARY

    faabric::transport::closeGlobalMessageContext();
    storage::shutdownFaasmS3();
    return result;
}
