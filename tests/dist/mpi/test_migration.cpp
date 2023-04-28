#include <catch2/catch.hpp>

#include "fixtures.h"

#include <faabric/scheduler/Scheduler.h>

#include <set>

namespace tests {

TEST_CASE_METHOD(MpiDistTestsFixture,
                 "Test migrating an MPI execution",
                 "[mpi]")
{
    // Under-allocate resources
    int mpiWorldSize = 4;
    setLocalRemoteSlots(2, 2);

    // Set up the message
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("mpi", "migrate", 1);
    faabric::Message& msg = req->mutable_messages()->at(0);
    msg.set_ismpi(true);
    msg.set_mpiworldsize(mpiWorldSize);

    // Try to migrate at 50% of execution
    int checkAt = 5;
    int numLoops = 10000;
    msg.set_cmdline(fmt::format("{} {}", checkAt, numLoops));

    // Call the functions
    sch.callFunctions(req);

    // Wait until the planner has had time to dispatch all the calls
    waitForMpiMessagesInFlight(req);

    // Get the scheduling decision now
    auto decision = sch.getPlannerClient()->getSchedulingDecision(req);

    // Update the slots so that a migration opportunity appears
    bool migrateMainRank;
    SECTION("Don't migrate main rank")
    {
        migrateMainRank = false;
        setLocalRemoteSlots(4, 2);
    }

    SECTION("Migrate main rank")
    {
        migrateMainRank = true;
        setLocalRemoteSlots(2, 4);
    }

    // Check the expected hosts before against the scheduling decision
    std::vector<std::string> expectedHostsBefore = { getDistTestMasterIp(),
                                                     getDistTestMasterIp(),
                                                     getDistTestWorkerIp(),
                                                     getDistTestWorkerIp() };
    REQUIRE(decision.hosts == expectedHostsBefore);

    std::vector<std::string> expectedHostsAfter;

    if (migrateMainRank) {
        // Check the expected hosts and message results at the end
        expectedHostsAfter = { getDistTestWorkerIp(),
                               getDistTestWorkerIp(),
                               getDistTestWorkerIp(),
                               getDistTestWorkerIp() };
    } else {
        // Check the expected hosts and message results at the end
        expectedHostsAfter = { getDistTestMasterIp(),
                               getDistTestMasterIp(),
                               getDistTestMasterIp(),
                               getDistTestMasterIp() };
    }
    checkMpiAllocationAndResult(req, expectedHostsAfter);
}
}
