#include <faabric/endpoint/FaabricEndpoint.h>
#include <faabric/runner/FaabricMain.h>
#include <faabric/transport/context.h>
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
        SPDLOG_INFO("Adding input data: {}", vm["input-data"].as<std::string>());
    }
    if (vm.count("cmdline")) {
        msg.set_cmdline(vm["cmdine"].as<std::string>());
        SPDLOG_INFO("Adding command line arguments: {}", vm["cmdine"].as<std::string>());
    }

    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    sch.callFunctions(req);

    for (const auto& m : req->messages()) {
        faabric::Message result = sch.getFunctionResult(m.id(), 20000);
        if (result.returnvalue() != 0) {
            SPDLOG_ERROR("Message ({}) returned error code: {}", m.id(), result.returnvalue());
            throw std::runtime_error("Message execution failed");
        }
    }

    return 0;
}

int main(int argc, char* argv[])
{
    storage::initFaasmS3();
    faabric::transport::initGlobalMessageContext();
    faabric::util::initLogging();

    // WARNING: All 0MQ-related operations must take place in a self-contined
    // scope to ensure all sockets are destructed before closing the context.
    {
        auto fac = std::make_shared<faaslet::FaasletFactory>();
        faabric::runner::FaabricMain m(fac);
        m.startRunner();

        // Start endpoint (will also have multiple threads)
        SPDLOG_INFO("Starting endpoint");
        faabric::endpoint::FaabricEndpoint endpoint;
        endpoint.start(faabric::endpoint::EndpointMode::SIGNAL);

        doRunner(argc, argv);

        SPDLOG_INFO("Shutting down");
        m.shutdown();

        faabric::transport::closeGlobalMessageContext();
    }

    storage::shutdownFaasmS3();
    return 0;
}
