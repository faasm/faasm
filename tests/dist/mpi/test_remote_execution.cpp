#include <catch2/catch.hpp>

#include "fixtures.h"

#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/ExecGraph.h>
#include <faabric/util/batch.h>

#include <set>

namespace tests {

TEST_CASE_METHOD(MpiDistTestsFixture,
                 "Test running an MPI function spanning two hosts",
                 "[mpi]")
{
    // Set up resources so that application is split among two hosts (two ranks
    // locally, two ranks remotely)
    int worldSize = 4;
    setLocalRemoteSlots(worldSize, 2, worldSize - 2, 0);

    // Set up the message
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("mpi", "mpi_bcast", 1);
    req->mutable_messages(0)->set_ismpi(true);
    req->mutable_messages(0)->set_mpiworldsize(worldSize);

    std::vector<std::string> expectedHosts = { getDistTestMasterIp(),
                                               getDistTestMasterIp(),
                                               getDistTestWorkerIp(),
                                               getDistTestWorkerIp() };

    // Call the functions
    plannerCli.callFunctions(req);
    waitForMpiMessagesInFlight(req);

    // Check it's successful
    checkMpiBatchResults(req, expectedHosts);
}
}
