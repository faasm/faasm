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

    sch.callFunctions(req);
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

    sch.callFunctions(req);
    faabric::Message result = plannerCli.getMessageResult(call, 5000);
    REQUIRE(result.returnvalue() == 0);
    for (const int msgId : faabric::util::getChainedFunctions(call)) {
        auto chainedResult =
          plannerCli.getMessageResult(call.appid(), msgId, 1000);
        REQUIRE(chainedResult.returnvalue() == 0);
    }

    auto execGraph = faabric::util::getFunctionExecGraph(result);
    int numNodes = faabric::util::countExecGraphNodes(execGraph);
    REQUIRE(numNodes == expectedNumNodes);
    REQUIRE(execGraph.rootNode.msg.id() == call.id());
}
}
