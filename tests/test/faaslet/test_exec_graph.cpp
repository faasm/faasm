#include <catch2/catch.hpp>

#include "faasm_fixtures.h"

#include <faabric/scheduler/ExecGraph.h>
#include <faabric/scheduler/Scheduler.h>

namespace tests {
TEST_CASE_METHOD(
  FunctionExecTestFixture,
  "Test function chaining can be recorded in the execution graph",
  "[exec-graph][chaining]")
{
    faabric::Message call =
      faabric::util::messageFactory("demo", "chain_named_a");
    int expectedNumNodes;

    SECTION("Turn recording on")
    {
        call.set_recordexecgraph(true);
        expectedNumNodes = 4;
    }

    SECTION("Recording off (default)") { expectedNumNodes = 1; }

    sch.callFunction(call);
    faabric::Message result = sch.getFunctionResult(call.id(), 5000);
    REQUIRE(result.returnvalue() == 0);

    auto execGraph = sch.getFunctionExecGraph(call.id());
    int numNodes = faabric::scheduler::countExecGraphNodes(execGraph);
    REQUIRE(numNodes == expectedNumNodes);
    REQUIRE(execGraph.rootNode.msg.id() == call.id());
}

TEST_CASE_METHOD(FunctionExecTestFixture,
                 "Test MPI executions can be recorded in the execution graph",
                 "[exec-graph][mpi]")
{
    faabric::Message call = faabric::util::messageFactory("mpi", "mpi_bcast");
    call.set_mpiworldsize(5);
    int expectedNumNodes;

    SECTION("Turn recording on")
    {
        call.set_recordexecgraph(true);
        expectedNumNodes = 5;
    }

    SECTION("Recording off (default)") { expectedNumNodes = 1; }

    sch.callFunction(call);
    faabric::Message result = sch.getFunctionResult(call.id(), 5000);
    REQUIRE(result.returnvalue() == 0);

    auto execGraph = sch.getFunctionExecGraph(call.id());
    int numNodes = faabric::scheduler::countExecGraphNodes(execGraph);
    REQUIRE(numNodes == expectedNumNodes);
    REQUIRE(execGraph.rootNode.msg.id() == call.id());
}
}
