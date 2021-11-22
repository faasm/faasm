#include <catch2/catch.hpp>

#include "fixtures.h"

#include <faabric/scheduler/Scheduler.h>

namespace tests {

TEST_CASE_METHOD(DistTestsFixture,
                 "Test running an MPI function spanning two hosts",
                 "[mpi]")
{
    // Set up this host's resources
    int nLocalSlots = 2;
    int mpiWorldSize = 4;
    faabric::HostResources res;
    res.set_slots(nLocalSlots);
    sch.setThisHostResources(res);

    // Set up the message
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("mpi", "mpi_bcast", 1);
    faabric::Message& msg = req->mutable_messages()->at(0);
    msg.set_ismpi(true);
    msg.set_mpiworldsize(mpiWorldSize);
    msg.set_recordexecgraph(true);

    // Call the functions
    sch.callFunctions(req);

    // Check it's successful
    faabric::Message result =
      sch.getFunctionResult(msg.id(), functionCallTimeout);
    REQUIRE(result.returnvalue() == 0);

    // Check exec graph
    auto execGraph = sch.getFunctionExecGraph(result.id());
    int numNodes = faabric::scheduler::countExecGraphNodes(execGraph);
    REQUIRE(numNodes == mpiWorldSize);
}
}
