#include <conf/FaasmConfig.h>
#include <conf/function_utils.h>
#include <faaslet/Faaslet.h>
#include <storage/S3Wrapper.h>
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
    bool isSgx = false;
    bool hasInput = false;
    if (argc == 4) {
        if (std::string(argv[3]) == "--sgx") {
            isSgx = true;
        } else {
            inputData = argv[3];
            hasInput = true;
        }
    } else if (argc == 5 && std::string(argv[4]) == "--sgx") {
        inputData = argv[3];
        hasInput = true;
        isSgx = true;
    }

    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory(user, function, 1);
    faabric::Message& msg = req->mutable_messages()->at(0);

    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    conf::FaasmConfig& faasmConf = conf::getFaasmConfig();

    // Set short timeouts to die quickly
    conf.boundTimeout = 60000;
    conf.globalMessageTimeout = 60000;
    faasmConf.chainedCallTimeout = 60000;

    // Make sure we have enough space for chained calls
    int nThreads = 10;
    faabric::HostResources res;
    res.set_slots(nThreads);
    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    sch.setThisHostResources(res);

    conf.defaultMpiWorldSize = 5;

    // Clear out redis
    faabric::redis::Redis& redis = faabric::redis::Redis::getQueue();
    redis.flushAll();

    if (user == "python") {
        conf::convertMessageToPython(msg);
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

    if (isSgx) {
        msg.set_issgx(true);
        SPDLOG_INFO("Running function in SGX");
    }

    // Set up the system
    auto fac = std::make_shared<faaslet::FaasletFactory>();
    faabric::runner::FaabricMain m(fac);
    m.startRunner();

    // Submit the invocation
    PROF_START(roundTrip)
    sch.callFunctions(req);

    // Await the result
    const faabric::Message& result =
      sch.getFunctionResult(msg.id(), conf.globalMessageTimeout);
    if (result.returnvalue() != 0) {
        SPDLOG_ERROR("Execution failed: {}", result.outputdata());
        throw std::runtime_error("Executing function failed");
    }

    PROF_END(roundTrip)

    m.shutdown();

    return 0;
}

int main(int argc, char* argv[])
{
    storage::initSDK();
    faabric::transport::initGlobalMessageContext();

    // WARNING: All 0MQ-related operations must take place in a self-contined
    // scope to ensure all sockets are destructed before closing the context.
    int result = doRunner(argc, argv);

    faabric::transport::closeGlobalMessageContext();
    storage::cleanUpSDK();
    return result;
}
