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
    int worldSize = 4;
    setLocalRemoteSlots(worldSize, worldSize);

    // Set up the message
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("mpi", "mpi_bcast", 1);
    req->mutable_messages(0)->set_ismpi(true);
    req->mutable_messages(0)->set_mpiworldsize(worldSize);

    std::vector<std::string> expectedHosts = { getDistTestMasterIp(),
                                               getDistTestMasterIp(),
                                               getDistTestWorkerIp(),
                                               getDistTestWorkerIp() };

    // Before calling the function, pre-load a scheduling decision so that
    // execution spans two hosts
    auto preloadDec = std::make_shared<batch_scheduler::SchedulingDecision>(
      req->appid(), req->groupid());
    for (int i = 0; i < expectedHosts.size(); i++) {
        preloadDec->addMessage(expectedHosts.at(i), 0, 0, i);
    }

    // Preload decision
    plannerCli.preloadSchedulingDecision(preloadDec);

    // Call the functions
    plannerCli.callFunctions(req);

    // Check it's successful
    checkMpiBatchResults(req, expectedHosts);
}
}
