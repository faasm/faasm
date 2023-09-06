#include <catch2/catch.hpp>

#include "fixtures.h"

#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/ExecGraph.h>
#include <faabric/util/batch.h>

#include <set>

namespace tests {

TEST_CASE_METHOD(MpiDistTestsFixture,
                 "Test migrating an MPI execution",
                 "[mpi]")
{
    // Allocate resources so that two mpi ranks are scheduled in one worker
    // and two in the other
    int worldSize = 4;
    // Give more total slots to the main so that the planner prefers it
    setLocalRemoteSlots(2 * worldSize, worldSize, 2 * worldSize - 2, 2);

    // Set up the message
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("mpi", "migrate", 1);
    faabric::Message& msg = req->mutable_messages()->at(0);
    msg.set_ismpi(true);
    msg.set_mpiworldsize(worldSize);
    msg.set_recordexecgraph(true);

    // Try to migrate at 50% of execution
    int numLoops = 10000;
    int checkAt = 5;
    msg.set_cmdline(fmt::format("{} {}", checkAt, numLoops));

    // Call the functions
    plannerCli.callFunctions(req);

    // Wait until all messages are in flight
    waitForMpiMessagesInFlight(req);

    // Update the total slots so that a migration opportunity appears. We
    // either migrate the first two ranks from the main to the worker, or
    // the other way around
    std::vector<std::string> expectedHostsAfter;

    SECTION("Migrate main rank")
    {
        setLocalRemoteSlots(2 * worldSize, worldSize, 2 * worldSize, 2);
        expectedHostsAfter = { getDistTestWorkerIp(),
                               getDistTestWorkerIp(),
                               getDistTestWorkerIp(),
                               getDistTestWorkerIp() };
    }

    SECTION("Do not migrate main rank")
    {
        setLocalRemoteSlots(2 * worldSize, worldSize, 2 * worldSize - 2, 2);
        expectedHostsAfter = { getDistTestMasterIp(),
                               getDistTestMasterIp(),
                               getDistTestMasterIp(),
                               getDistTestMasterIp() };
    }

    // Check it's successful
    auto result = getMpiBatchResult(msg);

    // Check that we have indeed migrated
    auto execGraph = faabric::util::getFunctionExecGraph(msg);
    std::vector<std::string> expectedHostsBefore = { getDistTestMasterIp(),
                                                     getDistTestMasterIp(),
                                                     getDistTestWorkerIp(),
                                                     getDistTestWorkerIp() };
    checkSchedulingFromExecGraph(
      execGraph, expectedHostsBefore, expectedHostsAfter);
}

TEST_CASE_METHOD(MpiDistTestsFixture,
                 "Test migrating two concurrent MPI applications",
                 "[mpi]")
{
    int worldSize = 4;
    int numLoops = 10000;
    int checkAt = 5;

    // Set up both requests
    std::shared_ptr<faabric::BatchExecuteRequest> reqA =
      faabric::util::batchExecFactory("mpi", "migrate", 1);
    reqA->mutable_messages(0)->set_ismpi(true);
    reqA->mutable_messages(0)->set_mpiworldsize(worldSize);
    reqA->mutable_messages(0)->set_recordexecgraph(true);
    reqA->mutable_messages(0)->set_cmdline(
      fmt::format("{} {}", checkAt, numLoops));

    std::shared_ptr<faabric::BatchExecuteRequest> reqB =
      faabric::util::batchExecFactory("mpi", "migrate", 1);
    reqB->mutable_messages(0)->set_ismpi(true);
    reqB->mutable_messages(0)->set_mpiworldsize(worldSize);
    reqB->mutable_messages(0)->set_recordexecgraph(true);
    reqB->mutable_messages(0)->set_cmdline(
      fmt::format("{} {}", checkAt, numLoops));

    // Allocate resources so that both applications are scheduled in the
    // same way: two ranks locally, and two remotely
    setLocalRemoteSlots(2 * worldSize, worldSize, 2 * worldSize - 2, 2);
    plannerCli.callFunctions(reqA);
    waitForMpiMessagesInFlight(reqA);

    setLocalRemoteSlots(2 * worldSize, worldSize, 2 * worldSize - 2, 2);
    plannerCli.callFunctions(reqB);
    waitForMpiMessagesInFlight(reqB);

    // Update the total slots so that two migration opportunities appear. Note
    // that when the first app is migrated (from worker to main) it will free
    // two slots in the worker that will be used by the second app to migrate
    // from main to worker
    setLocalRemoteSlots(
      3 * worldSize, 2 * worldSize, 3 * worldSize, 2 * worldSize - 2);

    std::vector<std::string> expectedHostsAfterA(worldSize,
                                                 getDistTestWorkerIp());
    std::vector<std::string> expectedHostsAfterB(worldSize,
                                                 getDistTestMasterIp());

    // Check it's successful
    getMpiBatchResult(reqA->messages(0));
    getMpiBatchResult(reqB->messages(0));

    // Check that we have indeed migrated
    auto execGraphA = faabric::util::getFunctionExecGraph(reqA->messages(0));
    auto execGraphB = faabric::util::getFunctionExecGraph(reqB->messages(0));
    std::vector<std::string> expectedHostsBefore = { getDistTestMasterIp(),
                                                     getDistTestMasterIp(),
                                                     getDistTestWorkerIp(),
                                                     getDistTestWorkerIp() };

    checkSchedulingFromExecGraph(
      execGraphA, expectedHostsBefore, expectedHostsAfterA);
    checkSchedulingFromExecGraph(
      execGraphB, expectedHostsBefore, expectedHostsAfterB);
}
}
