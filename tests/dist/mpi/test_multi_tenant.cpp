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

    // Set enough resources to fit both applications
    setLocalRemoteSlots(worldSize, worldSize, 0, 0);
    std::vector<std::string> expectedHostsA(4, "");
    std::vector<std::string> expectedHostsB(4, "");

    // Prepare both requests
    auto reqA = faabric::util::batchExecFactory("mpi", "mpi_long_alltoall", 1);
    reqA->mutable_messages(0)->set_ismpi(true);
    reqA->mutable_messages(0)->set_mpiworldsize(worldSize);
    auto reqB = faabric::util::batchExecFactory("mpi", "mpi_long_alltoall", 1);
    reqB->mutable_messages(0)->set_ismpi(true);
    reqB->mutable_messages(0)->set_mpiworldsize(worldSize);

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
        expectedHostsB = { getDistTestMasterIp(),
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

TEST_CASE_METHOD(
  MpiDistTestsFixture,
  "Test running an MPI and an OpenMP application at the same time",
  "[mpi]")
{
    // 11/03/2024 - OpenMP does not work with WAVM anymore
    if (faasmConf.wasmVm == "wavm") {
        return;
    }

    // We will distribute the MPI application across two hosts, and run an
    // OpenMP application in one of the hosts (not distributed)
    int mpiWorldSize = 4;
    // These numbers is hardcoded in the repeated_reduce function
    int ompNumThreads = 10;
    int ompNumLoops = 10;

    // Set enough resources to fit both applications. Note that, unfortunately,
    // we can not easily change the thread pool size in the remote world, so
    // we are stuck with the env. variable of `OVERRIDE_CPU_COUNT`. To this
    // extent, and given that the OpenMp function requires 10 threads, we can
    // only run it in the local host
    setLocalRemoteSlots(mpiWorldSize + ompNumThreads, mpiWorldSize, 0, 0);
    std::vector<std::string> expectedHostsA(mpiWorldSize, "");
    std::vector<std::string> expectedHostsB(ompNumThreads, "");

    // Prepare both requests
    auto reqA = faabric::util::batchExecFactory("mpi", "mpi_long_alltoall", 1);
    reqA->mutable_messages(0)->set_ismpi(true);
    reqA->mutable_messages(0)->set_mpiworldsize(mpiWorldSize);
    auto reqB = faabric::util::batchExecFactory("omp", "repeated_reduce", 1);

    // We need to be careful as OpenMP can only scale up locally
    SECTION("Concurrent")
    {
        expectedHostsA = { getDistTestMasterIp(),
                           getDistTestMasterIp(),
                           getDistTestWorkerIp(),
                           getDistTestWorkerIp() };
    }

    SECTION("Each one in a different host")
    {
        expectedHostsA = { getDistTestWorkerIp(),
                           getDistTestWorkerIp(),
                           getDistTestWorkerIp(),
                           getDistTestWorkerIp() };
    }

    // Preload scheduling decision for MPI application
    auto preloadDecA = std::make_shared<batch_scheduler::SchedulingDecision>(
      reqA->appid(), reqA->groupid());
    for (int i = 0; i < mpiWorldSize; i++) {
        preloadDecA->addMessage(expectedHostsA.at(i), 0, 0, i);
    }
    plannerCli.preloadSchedulingDecision(preloadDecA);

    plannerCli.callFunctions(reqA);
    waitForMpiMessagesInFlight(reqA);

    plannerCli.callFunctions(reqB);

    // Check both results are successful
    checkMpiBatchResults(reqA, expectedHostsA);

    // We do 10 loops with 10 threads, but the main thread only sets one
    // result, so we wait for a total of 91 messages
    auto ompBatchResults =
      waitForBatchResults(reqB, ompNumThreads * (ompNumLoops - 1) + 1);
    for (const auto& msg : ompBatchResults->messageresults()) {
        REQUIRE(msg.returnvalue() == 0);
        REQUIRE(msg.executedhost() == getDistTestMasterIp());
    }
}
}
