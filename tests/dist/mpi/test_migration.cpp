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

    // Try to migrate at 50% of execution
    int numLoops = 10000;
    int checkAt = 5;
    msg.set_cmdline(fmt::format("{} {}", checkAt, numLoops));

    // Call the functions
    plannerCli.callFunctions(req);

    // Wait until all messages are in flight
    std::vector<std::string> expectedHostsBefore = { getDistTestMasterIp(),
                                                     getDistTestMasterIp(),
                                                     getDistTestWorkerIp(),
                                                     getDistTestWorkerIp() };
    auto actualHostsBefore = waitForMpiMessagesInFlight(req);
    REQUIRE(expectedHostsBefore == actualHostsBefore);

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
    checkMpiBatchResults(req, expectedHostsAfter);
}

TEST_CASE_METHOD(MpiDistTestsFixture,
                 "Test triggering an MPI migration with a preloaded decision",
                 "[mpi]")
{
    // Allocate resources so that two mpi ranks are scheduled in one worker
    // and two in the other
    int worldSize = 4;
    // Give enough slots for the application to run in any of the two hosts
    setLocalRemoteSlots(2 * worldSize, worldSize, worldSize, 0);

    // Set up the message
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("mpi", "migrate", 1);
    faabric::Message& msg = req->mutable_messages()->at(0);
    msg.set_ismpi(true);
    msg.set_mpiworldsize(worldSize);

    // Try to migrate at 50% of execution
    int numLoops = 10000;
    int checkAt = 5;
    msg.set_cmdline(fmt::format("{} {}", checkAt, numLoops));

    // Before calling the function, pre-load a scheduling decision
    auto preloadDec = std::make_shared<batch_scheduler::SchedulingDecision>(
      req->appid(), req->groupid());

    std::vector<std::string> hostsBefore;
    std::vector<std::string> hostsAfter;

    SECTION("Do not migrate main rank")
    {
        hostsBefore = { getDistTestMasterIp(),
                        getDistTestMasterIp(),
                        getDistTestWorkerIp(),
                        getDistTestWorkerIp() };
        hostsAfter = { getDistTestMasterIp(),
                       getDistTestMasterIp(),
                       getDistTestMasterIp(),
                       getDistTestMasterIp() };
    }

    SECTION("Migrate main rank")
    {
        hostsBefore = { getDistTestMasterIp(),
                        getDistTestWorkerIp(),
                        getDistTestWorkerIp(),
                        getDistTestWorkerIp() };
        hostsAfter = { getDistTestWorkerIp(),
                       getDistTestWorkerIp(),
                       getDistTestWorkerIp(),
                       getDistTestWorkerIp() };
    }

    assert(hostsBefore.size() == worldSize);
    assert(hostsAfter.size() == worldSize);

    // In a preloaded scheduling decision we only care about the host we
    // execute at, and the group index
    for (int i = 0; i < hostsBefore.size(); i++) {
        preloadDec->addMessage(hostsBefore.at(i), 0, 0, i);
    }

    // Preload decision
    plannerCli.preloadSchedulingDecision(preloadDec);

    // Call the functions
    plannerCli.callFunctions(req);

    // Check it's successful
    checkMpiBatchResults(req, hostsAfter);
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
    reqA->mutable_messages(0)->set_cmdline(
      fmt::format("{} {}", checkAt, numLoops));

    std::shared_ptr<faabric::BatchExecuteRequest> reqB =
      faabric::util::batchExecFactory("mpi", "migrate", 1);
    reqB->mutable_messages(0)->set_ismpi(true);
    reqB->mutable_messages(0)->set_mpiworldsize(worldSize);
    reqB->mutable_messages(0)->set_cmdline(
      fmt::format("{} {}", checkAt, numLoops));

    // Allocate enough resources to run both applications in just one host.
    // We will still migrate both to improve locality
    setLocalRemoteSlots(2 * worldSize, 2 * worldSize, 0, 0);

    std::vector<std::string> expectedHostsBeforeA = { getDistTestMasterIp(),
                                                      getDistTestMasterIp(),
                                                      getDistTestMasterIp(),
                                                      getDistTestWorkerIp() };

    std::vector<std::string> expectedHostsBeforeB = { getDistTestMasterIp(),
                                                      getDistTestWorkerIp(),
                                                      getDistTestWorkerIp(),
                                                      getDistTestWorkerIp() };
    assert(expectedHostsBeforeA.size() == expectedHostsBeforeB.size());
    assert(expectedHostsBeforeA.size() == worldSize);

    // Preload scheduling decisions so that both applications are scheduled
    // in the same way:
    auto preloadDecA = std::make_shared<batch_scheduler::SchedulingDecision>(
      reqA->appid(), reqA->groupid());
    auto preloadDecB = std::make_shared<batch_scheduler::SchedulingDecision>(
      reqB->appid(), reqB->groupid());
    for (int i = 0; i < worldSize; i++) {
        preloadDecA->addMessage(expectedHostsBeforeA.at(i), 0, 0, i);
        preloadDecB->addMessage(expectedHostsBeforeB.at(i), 0, 0, i);
    }
    plannerCli.preloadSchedulingDecision(preloadDecA);
    plannerCli.preloadSchedulingDecision(preloadDecB);

    plannerCli.callFunctions(reqA);
    auto actualHostsBeforeA = waitForMpiMessagesInFlight(reqA);
    REQUIRE(actualHostsBeforeA == expectedHostsBeforeA);

    plannerCli.callFunctions(reqB);
    auto actualHostsBeforeB = waitForMpiMessagesInFlight(reqB);
    REQUIRE(actualHostsBeforeB == expectedHostsBeforeB);

    std::vector<std::string> expectedHostsAfterA(worldSize,
                                                 getDistTestMasterIp());
    std::vector<std::string> expectedHostsAfterB(worldSize,
                                                 getDistTestWorkerIp());

    // Check it's successful
    checkMpiBatchResults(reqA, expectedHostsAfterA);
    checkMpiBatchResults(reqB, expectedHostsAfterB);
}
}
