#include <catch2/catch.hpp>

#include "fixtures.h"

#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/ExecGraph.h>
#include <faabric/util/batch.h>

namespace tests {

TEST_CASE_METHOD(MpiDistTestsFixture,
                 "Test running two MPI applications at the same time",
                 "[mpi]")
{
    int worldSize = 4;

    // Prepare both requests
    std::shared_ptr<faabric::BatchExecuteRequest> reqA =
      faabric::util::batchExecFactory("mpi", "mpi_long_alltoall", 1);
    reqA->mutable_messages(0)->set_ismpi(true);
    reqA->mutable_messages(0)->set_mpiworldsize(worldSize);
    reqA->mutable_messages(0)->set_recordexecgraph(true);
    std::shared_ptr<faabric::BatchExecuteRequest> reqB =
      faabric::util::batchExecFactory("mpi", "mpi_long_alltoall", 1);
    reqB->mutable_messages(0)->set_ismpi(true);
    reqB->mutable_messages(0)->set_mpiworldsize(worldSize);
    reqB->mutable_messages(0)->set_recordexecgraph(true);

    // Allocate resources so that both applications are scheduled in the
    // same way: two ranks locally and two remotely
    setLocalRemoteSlots(2 * worldSize, worldSize, 2 * worldSize - 2, 2);
    plannerCli.callFunctions(reqA);
    waitForMpiMessagesInFlight(reqA);

    setLocalRemoteSlots(2 * worldSize, worldSize, 2 * worldSize - 2, 2);
    plannerCli.callFunctions(reqB);
    waitForMpiMessagesInFlight(reqB);

    // Check both results are successful
    auto resultA = getMpiBatchResult(reqA->messages(0));
    auto resultB = getMpiBatchResult(reqB->messages(0));

    // Get the execution graph for both requests
    auto execGraphA = faabric::util::getFunctionExecGraph(resultA);
    auto execGraphB = faabric::util::getFunctionExecGraph(resultB);

    // Builld the expectation for both requests
    std::vector<std::string> expectedHosts = { getDistTestMasterIp(),
                                               getDistTestMasterIp(),
                                               getDistTestWorkerIp(),
                                               getDistTestWorkerIp() };

    // Check the expecation against the actual execution graphs
    checkSchedulingFromExecGraph(execGraphA, expectedHosts);
    checkSchedulingFromExecGraph(execGraphB, expectedHosts);
}
}
