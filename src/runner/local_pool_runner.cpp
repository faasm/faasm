#include <faabric/endpoint/FaabricEndpoint.h>
#include <faabric/runner/FaabricMain.h>
#include <faabric/util/logging.h>
#include <faaslet/Faaslet.h>
#include <runner/runner_utils.h>
#include <storage/S3Wrapper.h>

int doRunner(int argc, char* argv[])
{
    auto vm = runner::parseRunnerCmdLine(argc, argv);
    std::string user = vm["user"].as<std::string>();
    std::string function = vm["function"].as<std::string>();
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory(user, function, 1);
    faabric::Message& msg = req->mutable_messages()->at(0);
    msg.set_topologyhint("FORCE_LOCAL");

    if (vm.count("input-data")) {
        msg.set_inputdata(vm["input-data"].as<std::string>());
    }
    if (vm.count("cmdline")) {
        msg.set_cmdline(vm["cmdline"].as<std::string>());
    }

    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    sch.callFunctions(req);

    SLEEP_MS(500);

    auto reqResponse = sch.getBatchResult(req, 20000);
    for (const auto& m : reqResponse->messages()) {
        if (m.returnvalue() != 0) {
            SPDLOG_ERROR(
              "Message ({}) returned error code: {}", m.id(), m.returnvalue());
            throw std::runtime_error("Message execution failed");
        }
    }

    return 0;
}

int main(int argc, char* argv[])
{
    storage::initFaasmS3();
    faabric::util::initLogging();

    auto& sch = faabric::scheduler::getScheduler();
    sch.shutdown();
    sch.addHostToGlobalSet();

    // Set timeout to ensure longer functions can finish
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    conf::FaasmConfig& faasmConf = conf::getFaasmConfig();
    conf.boundTimeout = 120000 * 100;
    conf.globalMessageTimeout = 120000 * 100;
    faasmConf.chainedCallTimeout = 120000 * 100;

    // Start the main faabric runner
    auto fac = std::make_shared<faaslet::FaasletFactory>();
    faabric::runner::FaabricMain m(fac);
    m.startBackground();

    doRunner(argc, argv);

    // Shutdown
    m.shutdown();
    storage::shutdownFaasmS3();

    return 0;
}
