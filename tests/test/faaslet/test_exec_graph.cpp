#include <catch2/catch.hpp>

#include "faasm_fixtures.h"

#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/ExecGraph.h>

namespace tests {
static std::vector<int> waitForChainedCalls(
  std::shared_ptr<BatchExecuteRequest> req,
  int expectedNumMsg)
{
    auto& plannerCli = faabric::planner::getPlannerClient();

    // Irrespective of whether we keep track of the execution in the exec.
    // graph, wait for all MPI messages
    int maxRetries = 10;
    int numRetries = 0;
    auto decision = plannerCli.getSchedulingDecision(req);
    while (decision.messageIds.size() != expectedNumMsg) {
        if (numRetries >= maxRetries) {
            SPDLOG_ERROR(
              "Timed-out waiting for chained messages to be scheduled ({}/{})",
              decision.messageIds.size(),
              expectedNumMsg);
            throw std::runtime_error("Timed-out waiting for MPI messges");
        }

        SPDLOG_DEBUG(
          "Waiting for chained messages to be scheduled ({}/{}, app: {})",
          decision.messageIds.size(),
          expectedNumMsg,
          req->appid());
        SLEEP_MS(1000);

        numRetries += 1;
        decision = plannerCli.getSchedulingDecision(req);

        // If the decision has no app ID, it means that the app has
        // already finished
        if (decision.appId == 0) {
            auto berStatus = plannerCli.getBatchResults(req);
            std::vector<int> messageIds;
            for (const auto& msg : berStatus->messageresults()) {
                messageIds.push_back(msg.id());
            }
            return messageIds;
        }
    }

    return decision.messageIds;
}

TEST_CASE_METHOD(
  FunctionExecTestFixture,
  "Test function chaining can be recorded in the execution graph",
  "[exec-graph][chaining]")
{
    auto req = faabric::util::batchExecFactory("demo", "chain_named_a", 1);
    auto& call = *req->mutable_messages(0);
    int expectedNumNodes;

    SECTION("Turn recording on")
    {
        call.set_recordexecgraph(true);
        expectedNumNodes = 4;
    }

    SECTION("Recording off (default)") { expectedNumNodes = 1; }

    plannerCli.callFunctions(req);
    auto chainedMessageIds = waitForChainedCalls(req, 4);

    auto firstResult = plannerCli.getMessageResult(call, 5000);
    for (const auto& mid : chainedMessageIds) {
        auto result = plannerCli.getMessageResult(call.appid(), mid, 1000);
        REQUIRE(result.returnvalue() == 0);
    }

    auto execGraph = faabric::util::getFunctionExecGraph(call);
    int numNodes = faabric::util::countExecGraphNodes(execGraph);
    REQUIRE(numNodes == expectedNumNodes);
    REQUIRE(execGraph.rootNode.msg.id() == call.id());
}

TEST_CASE_METHOD(FunctionExecTestFixture,
                 "Test MPI executions can be recorded in the execution graph",
                 "[exec-graph][mpi]")
{
    int mpiWorldSize = 5;
    auto req = faabric::util::batchExecFactory("mpi", "mpi_bcast", 1);
    auto call = req->messages(0);
    req->mutable_messages(0)->set_mpiworldsize(mpiWorldSize);
    int expectedNumNodes;

    SECTION("Turn recording on")
    {
        req->mutable_messages(0)->set_recordexecgraph(true);
        expectedNumNodes = mpiWorldSize;
    }

    SECTION("Recording off (default)") { expectedNumNodes = 1; }

    plannerCli.callFunctions(req);
    auto chainedMessageIds = waitForChainedCalls(req, mpiWorldSize);

    auto firstResult = plannerCli.getMessageResult(call, 5000);
    for (const auto& mid : chainedMessageIds) {
        auto result = plannerCli.getMessageResult(call.appid(), mid, 1000);
        REQUIRE(result.returnvalue() == 0);
    }

    auto execGraph = faabric::util::getFunctionExecGraph(firstResult);
    int numNodes = faabric::util::countExecGraphNodes(execGraph);
    REQUIRE(numNodes == expectedNumNodes);
    REQUIRE(execGraph.rootNode.msg.id() == call.id());
}
}
