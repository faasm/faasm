#include <faabric/endpoint/FaabricEndpoint.h>
#include <faabric/planner/PlannerClient.h>
#include <faabric/planner/PlannerServer.h>
#include <faabric/runner/FaabricMain.h>
#include <faabric/util/batch.h>
#include <faabric/util/logging.h>
#include <faabric/util/network.h>
#include <faaslet/Faaslet.h>
#include <runner/runner_utils.h>
#include <storage/S3Wrapper.h>

#define TIMEOUT_MS 60000

std::vector<faabric::Message> waitForBatchResults(bool isThreads,
                                                  int appId,
                                                  const std::set<int>& msgIds)
{
    auto& plannerCli = faabric::planner::getPlannerClient();

    std::vector<faabric::Message> resultMsgs;

    for (const auto& msgId : msgIds) {
        faabric::Message result =
          plannerCli.getMessageResult(appId, msgId, TIMEOUT_MS);
        resultMsgs.push_back(result);
    }

    return resultMsgs;
}

std::vector<faabric::Message> executeWithPool(
  std::shared_ptr<faabric::BatchExecuteRequest> req)
{
    bool isThreads = req->type() == faabric::BatchExecuteRequest::THREADS;

    std::set<int> reqMsgIds;
    int appId = req->messages(0).appid();
    for (const auto& msg : req->messages()) {
        reqMsgIds.insert(msg.id());
    }

    auto& plannerCli = faabric::planner::getPlannerClient();
    plannerCli.callFunctions(req);

    // In the case of an MPI request, we want to wait for all the MPI messages,
    // not only the one with rank 0
    if (req->messages(0).ismpi()) {
        int maxRetries = 5;
        int numRetries = 0;
        int expectedWorldSize = req->messages(0).mpiworldsize();
        auto decision = plannerCli.getSchedulingDecision(req);
        while (decision.messageIds.size() != expectedWorldSize) {
            if (numRetries >= maxRetries) {
                SPDLOG_ERROR(
                  "Timed-out waiting for MPI messages to be scheduled ({}/{})",
                  decision.messageIds.size(),
                  expectedWorldSize);
                throw std::runtime_error("Timed-out waiting for MPI messges");
            }

            SPDLOG_DEBUG(
              "Waiting for MPI messages to be scheduled ({}/{}, app: {})",
              decision.messageIds.size(),
              expectedWorldSize,
              req->appid());
            SLEEP_MS(1000);

            numRetries += 1;
            decision = plannerCli.getSchedulingDecision(req);

            // If the decision has no app ID, it means that the app has
            // already finished, so we don't even have to wait for the messages
            if (decision.appId == 0) {
                auto berStatus = plannerCli.getBatchResults(req);
                return std::vector<faabric::Message>(
                  berStatus->mutable_messageresults()->begin(),
                  berStatus->mutable_messageresults()->end());
            }
        }

        // Finally, add the message IDs to the waiting set
        for (const auto& mid : decision.messageIds) {
            reqMsgIds.insert(mid);
        }
    }

    // Wait for all functions to complete
    auto resultMsgs = waitForBatchResults(isThreads, appId, reqMsgIds);

    return resultMsgs;
}

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
    if (vm.count("mpi-world-size")) {
        msg.set_ismpi(true);
        msg.set_mpiworldsize(vm["mpi-world-size"].as<int>());
    }

    auto msgResults = executeWithPool(req);

    for (const auto& m : msgResults) {
        if (m.returnvalue() != 0) {
            SPDLOG_ERROR(
              "Message ({}) returned error code: {}", m.id(), m.returnvalue());
            return m.returnvalue();
        }
    }

    return 0;
}

int main(int argc, char* argv[])
{
    storage::initFaasmS3();
    faabric::util::initLogging();

    // First, manually start a planner in LOCALHOST
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    conf.plannerHost = LOCALHOST;
    faabric::planner::PlannerServer plannerServer;
    plannerServer.start();
    faabric::planner::getPlannerClient().ping();

    // Second, start a regular pool runner
    conf::getFaasmConfig().print();
    auto fac = std::make_shared<faaslet::FaasletFactory>();
    faabric::runner::FaabricMain m(fac);
    m.startBackground();

    // Third, actually run the request
    auto retVal = doRunner(argc, argv);

    // Clean-up
    m.shutdown();
    plannerServer.stop();
    storage::shutdownFaasmS3();

    return retVal;
}
