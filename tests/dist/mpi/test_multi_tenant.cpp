#include <catch2/catch.hpp>

#include "fixtures.h"

#include <faabric/scheduler/Scheduler.h>

namespace tests {

TEST_CASE_METHOD(MpiDistTestsFixture,
                 "Test running two MPI applications at the same time",
                 "[mpi]")
{
    // Set up this host's resources
    int worldSize = 4;
    setLocalRemoteSlots(2, 2);

    // Set up the message
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("mpi", "mpi_long_alltoall", 1);
    faabric::Message& msg = req->mutable_messages()->at(0);
    msg.set_ismpi(true);
    msg.set_mpiworldsize(worldSize);

    // Call the functions
    sch.callFunctions(req);

    // Wait until the planner has had time to dispatch all the calls
    waitForMpiMessagesInFlight(req);

    // Update the slots for the same scheduling decision
    setLocalRemoteSlots(2, 2);

    // Set up the second message
    std::shared_ptr<faabric::BatchExecuteRequest> reqCopy =
      faabric::util::batchExecFactory("mpi", "mpi_long_alltoall", 1);
    faabric::Message& msgCopy = reqCopy->mutable_messages()->at(0);
    msgCopy.set_ismpi(true);
    msgCopy.set_mpiworldsize(worldSize);

    // Call the functions for a second time
    sch.callFunctions(reqCopy);

    std::vector<std::string> expectedHosts = { getDistTestMasterIp(),
                                               getDistTestMasterIp(),
                                               getDistTestWorkerIp(),
                                               getDistTestWorkerIp() };
    // Check allocation and results for the first request
    checkMpiAllocationAndResult(req, expectedHosts);

    // Check allocation and result for the second request
    checkMpiAllocationAndResult(reqCopy, expectedHosts);
}
}
