#include <faabric/endpoint/FaabricEndpoint.h>
#include <faabric/planner/PlannerClient.h>
#include <faabric/runner/FaabricMain.h>
#include <faabric/util/batch.h>
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

    if (vm.count("input-data")) {
        msg.set_inputdata(vm["input-data"].as<std::string>());
    }
    if (vm.count("cmdline")) {
        msg.set_cmdline(vm["cmdline"].as<std::string>());
    }

    auto& plannerCli = faabric::planner::getPlannerClient();
    plannerCli.callFunctions(req);

    usleep(1000 * 500);

    for (const auto& m : req->messages()) {
        faabric::Message result = plannerCli.getMessageResult(m, 20000 * 100);
        if (result.returnvalue() != 0) {
            SPDLOG_ERROR("Message ({}) returned error code: {}",
                         m.id(),
                         result.returnvalue());
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

    // WARNING: All 0MQ-related operations must take place in a self-contined
    // scope to ensure all sockets are destructed before closing the context.
    {
        auto fac = std::make_shared<faaslet::FaasletFactory>();
        faabric::runner::FaabricMain m(fac);
        m.startRunner();

        doRunner(argc, argv);

        m.shutdown();
    }

    storage::shutdownFaasmS3();
    return 0;
}
