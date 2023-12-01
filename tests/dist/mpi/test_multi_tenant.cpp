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
    std::shared_ptr<faabric::BatchExecuteRequest> reqB =
      faabric::util::batchExecFactory("mpi", "mpi_long_alltoall", 1);
    reqB->mutable_messages(0)->set_ismpi(true);
    reqB->mutable_messages(0)->set_mpiworldsize(worldSize);

    // Set enough resources to fit both applications
    setLocalRemoteSlots(worldSize, worldSize, 0, 0);
    std::vector<std::string> expectedHostsA(4, 0);
    std::vector<std::string> expectedHostsB(4, 0);

    SECTION("Same distribution")
    {
        expectedHostsA = { getDistTestMasterIp(),
                           getDistTestMasterIp(),
                           getDistTestWorkerIp(),
                           getDistTestWorkerIp() };
        expectedHostsB = expectedHostsA;
    }

    SECTION("Inverted")
    {
        expectedHostsA = { getDistTestMasterIp(),
                           getDistTestMasterIp(),
                           getDistTestWorkerIp(),
                           getDistTestWorkerIp() };
        expectedHostsB = { getDistTestWorkerIp(),
                           getDistTestWorkerIp(),
                           getDistTestMasterIp(),
                           getDistTestMasterIp() };
    }

    SECTION("Uneven")
    {
        expectedHostsA = { getDistTestMasterIp(),
                           getDistTestMasterIp(),
                           getDistTestMasterIp(),
                           getDistTestWorkerIp() };
        expectedHostsA = { getDistTestMasterIp(),
                           getDistTestWorkerIp(),
                           getDistTestWorkerIp(),
                           getDistTestWorkerIp() };
    }

    // Preload scheduling decisions so that both applications are scheduled
    // in the same way: two ranks locally and two rankls remotely
    auto preloadDecA = std::make_shared<batch_scheduler::SchedulingDecision>(
      reqA->appid(), reqA->groupid());
    auto preloadDecB = std::make_shared<batch_scheduler::SchedulingDecision>(
      reqB->appid(), reqB->groupid());
    for (int i = 0; i < worldSize; i++) {
        preloadDecA->addMessage(expectedHostsA.at(i), 0, 0, i);
        preloadDecB->addMessage(expectedHostsB.at(i), 0, 0, i);
    }
    plannerCli.preloadSchedulingDecision(preloadDecA);
    plannerCli.preloadSchedulingDecision(preloadDecB);

    plannerCli.callFunctions(reqA);
    waitForMpiMessagesInFlight(reqA);

    plannerCli.callFunctions(reqB);
    waitForMpiMessagesInFlight(reqB);

    // Check both results are successful
    checkMpiBatchResults(reqA, expectedHostsA);
    checkMpiBatchResults(reqB, expectedHostsB);
}
}
