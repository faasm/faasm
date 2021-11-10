#include <catch2/catch.hpp>

#include "faasm_fixtures.h"

#include <faabric/runner/FaabricMain.h>
#include <faabric/scheduler/ExecGraph.h>
#include <faabric/scheduler/Scheduler.h>
#include <faaslet/Faaslet.h>

namespace tests {
class ExecGraphTestFixture : public FunctionExecTestFixture
{
  public:
    ExecGraphTestFixture()
      : m(std::make_shared<faaslet::FaasletFactory>())
    {
        // Modify system config (network ns requires root)
        auto& conf = faabric::util::getSystemConfig();
        conf.boundTimeout = 5000;

        m.startRunner();
    }

    ~ExecGraphTestFixture() { m.shutdown(); }

    faabric::Message callFunctionGetResult(faabric::Message& call,
                                           int timeout = 5000)
    {
        sch.callFunction(call);

        faabric::Message result = sch.getFunctionResult(call.id(), timeout);
        REQUIRE(result.returnvalue() == 0);
        return result;
    }

  protected:
    faabric::runner::FaabricMain m;
};

TEST_CASE_METHOD(
  ExecGraphTestFixture,
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

    faabric::Message result = callFunctionGetResult(call);

    auto execGraph = sch.getFunctionExecGraph(call.id());
    int numNodes = faabric::scheduler::countExecGraphNodes(execGraph);
    REQUIRE(numNodes == expectedNumNodes);
    REQUIRE(execGraph.rootNode.msg.id() == call.id());
}

TEST_CASE_METHOD(ExecGraphTestFixture,
                 "Test MPI executions can be recorded in the execution graph",
                 "[exec-graph][mpiT]")
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

    faabric::Message result = callFunctionGetResult(call);

    auto execGraph = sch.getFunctionExecGraph(call.id());
    int numNodes = faabric::scheduler::countExecGraphNodes(execGraph);
    REQUIRE(numNodes == expectedNumNodes);
    REQUIRE(execGraph.rootNode.msg.id() == call.id());
}
}
