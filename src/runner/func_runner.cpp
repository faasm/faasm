#include <conf/FaasmConfig.h>
#include <faabric/planner/PlannerClient.h>
#include <faabric/redis/Redis.h>
#include <faabric/runner/FaabricMain.h>
#include <faabric/scheduler/ExecutorFactory.h>
#include <faabric/util/batch.h>
#include <faabric/util/config.h>
#include <faabric/util/environment.h>
#include <faabric/util/logging.h>
#include <faabric/util/timing.h>
#include <faaslet/Faaslet.h>
#include <runner/runner_utils.h>
#include <storage/FileLoader.h>
#include <wasm/WasmModule.h>

namespace po = boost::program_options;

int doRunner(int argc, char* argv[])
{
    faabric::util::initLogging();

    auto vm = runner::parseRunnerCmdLine(argc, argv);
    std::string user = vm["user"].as<std::string>();
    std::string function = vm["function"].as<std::string>();

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

    if (vm.count("input-data")) {
        msg.set_inputdata(vm["input-data"].as<std::string>());
        SPDLOG_INFO("Adding input data: {}",
                    vm["input-data"].as<std::string>());
    }

    if (vm.count("cmdline")) {
        msg.set_cmdline(vm["cmdline"].as<std::string>());
        SPDLOG_INFO("Adding command line arguments: {}",
                    vm["cmdline"].as<std::string>());
    }

    // Set up the system
    auto fac = std::make_shared<faaslet::FaasletFactory>();
    faabric::runner::FaabricMain m(fac);
    m.startRunner();

    // Submit the invocation
    PROF_START(FunctionExec)
    auto& plannerCli = faabric::planner::getPlannerClient();
    plannerCli.callFunctions(req);

    // Await the result
    const faabric::Message& result =
      plannerCli.getMessageResult(msg, conf.globalMessageTimeout);
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
    storage::initFaasmS3();

    PROF_BEGIN

    // WARNING: All 0MQ-related operations must take place in a self-contined
    // scope to ensure all sockets are destructed before closing the context.
    int result = doRunner(argc, argv);

    PROF_SUMMARY

    storage::shutdownFaasmS3();
    return result;
}
