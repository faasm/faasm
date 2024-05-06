#include <catch2/catch.hpp>

#include "fixtures.h"

#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/batch.h>

namespace tests {

TEST_CASE_METHOD(MpiDistTestsFixture,
                 "Test triggering an MPI migration with a preloaded decision",
                 "[mpi]")
{
    updatePlannerPolicy("bin-pack");

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
    auto actualHostsBefore = waitForMpiMessagesInFlight(req);
    REQUIRE(actualHostsBefore == hostsBefore);

    // Check it's successful
    checkMpiBatchResults(req, hostsAfter);
}

TEST_CASE_METHOD(MpiDistTestsFixture,
                 "Test migrating an MPI application with Compact policy",
                 "[mpi]")
{
    updatePlannerPolicy("compact");

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
        hostsBefore = { getDistTestWorkerIp(),
                        getDistTestWorkerIp(),
                        getDistTestMasterIp(),
                        getDistTestMasterIp() };
        hostsAfter = { getDistTestMasterIp(),
                       getDistTestMasterIp(),
                       getDistTestMasterIp(),
                       getDistTestMasterIp() };
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
    auto actualHostsBefore = waitForMpiMessagesInFlight(req);
    REQUIRE(actualHostsBefore == hostsBefore);

    // Check it's successful
    checkMpiBatchResults(req, hostsAfter);

    updatePlannerPolicy("bin-pack");
}

TEST_CASE_METHOD(MpiDistTestsFixture,
                 "Test migrating an MPI application with SPOT policy",
                 "[mpi]")
{
    updatePlannerPolicy("spot");

    // Allocate resources so that two mpi ranks are scheduled in one worker
    // and two in the other
    int worldSize = 4;

    // Give enough slots for the application to run in any of the two hosts
    setLocalRemoteSlots(worldSize, worldSize);

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

    // Migrations with the SPOT policy are triggered by setting a VM as
    // evicted
    std::string evictedVmIp;

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
        evictedVmIp = getDistTestWorkerIp();
    }

    SECTION("Migrate main rank (partial app)")
    {
        hostsBefore = { getDistTestMasterIp(),
                        getDistTestMasterIp(),
                        getDistTestWorkerIp(),
                        getDistTestWorkerIp() };
        hostsAfter = { getDistTestWorkerIp(),
                       getDistTestWorkerIp(),
                       getDistTestWorkerIp(),
                       getDistTestWorkerIp() };
        evictedVmIp = getDistTestMasterIp();
    }

    SECTION("Migrate main rank (full app)")
    {
        hostsBefore = { getDistTestMasterIp(),
                        getDistTestMasterIp(),
                        getDistTestMasterIp(),
                        getDistTestMasterIp() };
        hostsAfter = { getDistTestWorkerIp(),
                       getDistTestWorkerIp(),
                       getDistTestWorkerIp(),
                       getDistTestWorkerIp() };
        evictedVmIp = getDistTestMasterIp();
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

    // Mark the VM as evicted (note that pre-loading takes preference, so we
    // can mark the VM as evicted before scheduling it)
    setNextEvictedVmIp({ evictedVmIp });

    // Call the functions
    plannerCli.callFunctions(req);

    // Check it's successful
    checkMpiBatchResults(req, hostsAfter);

    updatePlannerPolicy("bin-pack");
}

TEST_CASE_METHOD(MpiDistTestsFixture,
                 "Test migrating between two hosts without emptying neither",
                 "[mpi]")
{
    // Allocate resources so that two mpi ranks are scheduled in one worker
    // and two in the other
    int worldSize = 6;
    int numSlots = 4;

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

    // Force a sub-optimal scheduling (3 locally, three remotely), so that
    // we migrate one rank from one host to the other. Both hosts will stil
    // run ranks after the migration
    SECTION("From non-main to main")
    {
        setLocalRemoteSlots(numSlots, numSlots - 1, 0, 0);

        hostsBefore = { getDistTestMasterIp(), getDistTestMasterIp(),
                        getDistTestMasterIp(), getDistTestWorkerIp(),
                        getDistTestWorkerIp(), getDistTestWorkerIp() };
        hostsAfter = { getDistTestMasterIp(), getDistTestMasterIp(),
                       getDistTestMasterIp(), getDistTestWorkerIp(),
                       getDistTestWorkerIp(), getDistTestMasterIp() };
    }

    SECTION("From main to non-main")
    {
        // This test-case is particularly tricky as we need to chain the local
        // leader in the non-main host
        setLocalRemoteSlots(numSlots - 1, numSlots + 1, 0, 0);

        hostsBefore = { getDistTestMasterIp(), getDistTestMasterIp(),
                        getDistTestMasterIp(), getDistTestWorkerIp(),
                        getDistTestWorkerIp(), getDistTestWorkerIp() };
        hostsAfter = { getDistTestMasterIp(), getDistTestWorkerIp(),
                       getDistTestWorkerIp(), getDistTestWorkerIp(),
                       getDistTestWorkerIp(), getDistTestWorkerIp() };
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
    auto actualHostsBefore = waitForMpiMessagesInFlight(req);
    REQUIRE(actualHostsBefore == hostsBefore);

    // Check it's successful
    checkMpiBatchResults(req, hostsAfter);
}

TEST_CASE_METHOD(MpiDistTestsFixture,
                 "Test migrating two concurrent MPI applications (bin-pack)",
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

TEST_CASE_METHOD(MpiDistTestsFixture,
                 "Test migrating with concurrent MPI applications (compact)",
                 "[mpi]")
{
    updatePlannerPolicy("compact");

    int worldSize = 4;
    int numLoops = 10000;
    int checkAt = 5;

    // Request A will migrate
    std::shared_ptr<faabric::BatchExecuteRequest> reqA =
      faabric::util::batchExecFactory("mpi", "migrate", 1);
    reqA->mutable_messages(0)->set_ismpi(true);
    reqA->mutable_messages(0)->set_mpiworldsize(worldSize);
    reqA->mutable_messages(0)->set_cmdline(
      fmt::format("{} {}", checkAt, numLoops));

    // Request B will not migrate
    std::shared_ptr<faabric::BatchExecuteRequest> reqB =
      faabric::util::batchExecFactory("mpi", "mpi_long_alltoall", 1);
    reqB->mutable_messages(0)->set_ismpi(true);
    reqB->mutable_messages(0)->set_mpiworldsize(worldSize);
    reqB->mutable_messages(0)->set_cmdline(
      fmt::format("{} {}", numLoops, numLoops));

    // Allocate enough resources to run both applications in just one host.
    // We will still migrate one app to increase the number of free hosts
    setLocalRemoteSlots(2 * worldSize, 2 * worldSize, 0, 0);

    std::vector<std::string> expectedHostsBeforeA = { getDistTestMasterIp(),
                                                      getDistTestMasterIp(),
                                                      getDistTestMasterIp(),
                                                      getDistTestMasterIp() };

    std::vector<std::string> expectedHostsBeforeB = { getDistTestWorkerIp(),
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

    plannerCli.callFunctions(reqB);

    plannerCli.callFunctions(reqA);

    std::vector<std::string> expectedHostsAfterA(worldSize,
                                                 getDistTestWorkerIp());
    std::vector<std::string> expectedHostsAfterB = expectedHostsBeforeB;

    // Check it's successful
    checkMpiBatchResults(reqA, expectedHostsAfterA);
    checkMpiBatchResults(reqB, expectedHostsAfterB);

    updatePlannerPolicy("bin-pack");
}

TEST_CASE_METHOD(MpiDistTestsFixture,
                 "Test freezing an application with SPOT policy",
                 "[mpi]")
{
    updatePlannerPolicy("spot");

    // If we mark a VM as evicted, and try to migrate with the SPOT policy
    // but have nowhere to migrate to, the app will be frozen until slots
    // appear. The trigger to check if the app can be unfrozen is a query
    // to get the batch result (as part of a regular polling)

    int worldSize = 4;
    int numLoops = 10000;
    int checkAt = 5;

    // Request A will freeze
    std::shared_ptr<faabric::BatchExecuteRequest> reqA =
      faabric::util::batchExecFactory("mpi", "migrate", 1);
    reqA->mutable_messages(0)->set_ismpi(true);
    reqA->mutable_messages(0)->set_mpiworldsize(worldSize);
    reqA->mutable_messages(0)->set_cmdline(
      fmt::format("{} {}", checkAt, numLoops));

    // Request B will not freeze
    std::shared_ptr<faabric::BatchExecuteRequest> reqB =
      faabric::util::batchExecFactory("mpi", "mpi_long_alltoall", 1);
    reqB->mutable_messages(0)->set_ismpi(true);
    reqB->mutable_messages(0)->set_mpiworldsize(worldSize);
    reqB->mutable_messages(0)->set_cmdline(
      fmt::format("{} {}", numLoops, numLoops));

    // Allocate resources so that when we evict one VM, there's not enough
    // slots to run both apps in one VM
    setLocalRemoteSlots(worldSize, worldSize);

    std::vector<std::string> expectedHostsBeforeA = { getDistTestWorkerIp(),
                                                      getDistTestWorkerIp(),
                                                      getDistTestWorkerIp(),
                                                      getDistTestWorkerIp() };

    std::vector<std::string> expectedHostsBeforeB = { getDistTestMasterIp(),
                                                      getDistTestMasterIp(),
                                                      getDistTestMasterIp(),
                                                      getDistTestMasterIp() };

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

    // Mark the worker VM as evicted
    std::string evictedVmIp = getDistTestWorkerIp();
    setNextEvictedVmIp({ evictedVmIp });

    plannerCli.callFunctions(reqB);

    plannerCli.callFunctions(reqA);

    // First, we wait for the non-migrated app to finish. In the process,
    // the app in the evicted VM will have FROZEN
    checkMpiBatchResults(reqB, expectedHostsBeforeB);

    // Second, we can check that there are no apps in-flight, but one is
    // frozen
    auto inFlightApps = getInFlightApps();
    REQUIRE(inFlightApps.apps_size() == 0);
    REQUIRE(inFlightApps.frozenapps_size() == 1);
    REQUIRE(inFlightApps.frozenapps(0).appid() == reqA->appid());

    // Third, we can try to get the batch results to trigger an un-freeze
    auto batchResultsA = plannerCli.getBatchResults(reqA);
    REQUIRE(!batchResultsA->finished());

    // Finally, we can wait on the un-frozen app to finish
    auto expectedHostsAfterA =
      std::vector<std::string>(worldSize, getDistTestMasterIp());
    checkMpiBatchResults(reqA, expectedHostsAfterA);

    updatePlannerPolicy("bin-pack");
}
}
