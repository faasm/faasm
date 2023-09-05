#include <catch2/catch.hpp>

#include "faasm_fixtures.h"

#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/ExecGraph.h>

namespace tests {
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
    faabric::Message result = plannerCli.getMessageResult(call, 5000);
    REQUIRE(result.returnvalue() == 0);

    auto execGraph = faabric::util::getFunctionExecGraph(call);
    int numNodes = faabric::util::countExecGraphNodes(execGraph);
    REQUIRE(numNodes == expectedNumNodes);
    REQUIRE(execGraph.rootNode.msg.id() == call.id());
}

TEST_CASE_METHOD(FunctionExecTestFixture,
                 "Test MPI executions can be recorded in the execution graph",
                 "[exec-graph][mpi]")
{
    auto req = faabric::util::batchExecFactory("mpi", "mpi_bcast", 1);
    auto call = req->messages(0);
    req->mutable_messages(0)->set_mpiworldsize(5);
    int expectedNumNodes;

    SECTION("Turn recording on")
    {
        req->mutable_messages(0)->set_recordexecgraph(true);
        expectedNumNodes = 5;
    }

    SECTION("Recording off (default)") { expectedNumNodes = 1; }

    plannerCli.callFunctions(req);

    // Irrespective of whether we keep track of the execution in the exec.
    // graph, wait for all MPI messages
    int maxRetries = 10;
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

        SPDLOG_DEBUG("Waiting for MPI messages to be scheduled ({}/{})",
                     decision.messageIds.size(),
                     expectedWorldSize);
        SLEEP_MS(500);

        numRetries += 1;
        decision = plannerCli.getSchedulingDecision(req);
    }

    auto firstResult = plannerCli.getMessageResult(call, 5000);
    for (const auto& mid : decision.messageIds) {
        auto result = plannerCli.getMessageResult(call.appid(), mid, 1000);
        REQUIRE(result.returnvalue() == 0);
    }

    auto execGraph = faabric::util::getFunctionExecGraph(firstResult);
    int numNodes = faabric::util::countExecGraphNodes(execGraph);
    REQUIRE(numNodes == expectedNumNodes);
    REQUIRE(execGraph.rootNode.msg.id() == call.id());
}
}
