#include <conf/FaasmConfig.h>
#include <faabric/redis/Redis.h>
#include <faabric/runner/FaabricMain.h>
#include <faabric/scheduler/ExecutorFactory.h>
#include <faabric/transport/context.h>
#include <faabric/util/config.h>
#include <faabric/util/environment.h>
#include <faabric/util/logging.h>
#include <faabric/util/timing.h>
#include <faaslet/Faaslet.h>
#include <storage/FileLoader.h>
#include <wasm/WasmModule.h>

#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;

int doRunner(int argc, char* argv[])
{
    faabric::util::initLogging();

    // Define command line arguments
    std::string user;
    std::string function;
    std::string inputData;
    std::string cmdLine;
    po::options_description desc("Allowed options");
    desc.add_options()(
      "user", po::value<std::string>(&user), "function's user name (required)")(
      "function",
      po::value<std::string>(&function),
      "function name (required)")("input-data",
                                  po::value<std::string>(&inputData),
                                  "input data for the function")(
      "cmdline",
      po::value<std::string>(&cmdLine),
      "command line arguments to pass the function");

    // Mark user and function as positional arguments
    po::positional_options_description p;
    p.add("user", 1);
    p.add("function", 2);

    // Parse command line arguments
    po::variables_map vm;
    po::store(
      po::command_line_parser(argc, argv).options(desc).positional(p).run(),
      vm);
    po::notify(vm);

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
        msg.set_inputdata(inputData);
        SPDLOG_INFO("Adding input data: {}", inputData);
    }

    if (vm.count("cmdline")) {
        msg.set_cmdline(cmdLine);
        SPDLOG_INFO("Adding command line arguments: {}", cmdLine);
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
