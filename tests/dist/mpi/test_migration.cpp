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

    // Sleep for a bit to let the scheduler schedule the MPI calls, and then
    // update the local slots so that a migration opportunity appears. We
    // either migrate the first two ranks from the main to the worker, or
    // the other way around
    SLEEP_MS(200);
    bool migrateMainRank = false;
    std::vector<std::string> expectedHostsAfter;

    /*
    SECTION("Migrate main rank")
    {
        migrateMainRank = true;
        setLocalRemoteSlots(2 * worldSize, worldSize, 2 * worldSize - 2, 2);
    }
    */

    SECTION("Do not migrate main rank")
    {
        migrateMainRank = false;
        setLocalRemoteSlots(2 * worldSize, worldSize, 2 * worldSize - 2, 2);
        expectedHostsAfter = { getDistTestMasterIp(),
                               getDistTestMasterIp(),
                               getDistTestMasterIp(),
                               getDistTestMasterIp() };
    }

    // Check it's successful
    auto result = getMpiBatchResult(msg, true);

    // Check that we have indeed migrated
    auto execGraph = faabric::util::getFunctionExecGraph(msg);
    std::vector<std::string> expectedHostsBefore = { getDistTestMasterIp(),
                                                     getDistTestMasterIp(),
                                                     getDistTestWorkerIp(),
                                                     getDistTestWorkerIp() };
    checkSchedulingFromExecGraph(
      execGraph, expectedHostsBefore, expectedHostsAfter);
}

/*
TEST_CASE_METHOD(MpiDistTestsFixture,
                 "Test forcing an MPI migration through a topology hint",
                 "[mpi]")
{
    // Set enough slots locally to run all functions, but UNDERFULL topology
    // hint will overwrite force a sub-optimal scheduling
    int mpiWorldSize = 4;
    faabric::HostResources res;
    res.set_slots(mpiWorldSize);
    sch.setThisHostResources(res);

    // Set up the message (important to set the topologyhint)
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("mpi", "migrate", 1);
    faabric::Message& msg = req->mutable_messages()->at(0);
    msg.set_ismpi(true);
    msg.set_mpiworldsize(mpiWorldSize);
    msg.set_recordexecgraph(true);
    msg.set_topologyhint("UNDERFULL");

    // Set a low migration check period to detect the mgiration right away
    int migrationCheckPeriod = 1;
    msg.set_migrationcheckperiod(migrationCheckPeriod);

    // Try to migrate at 50% of execution
    int checkAt = 5;
    int numLoops = 10000;
    msg.set_cmdline(fmt::format("{} {}", checkAt, numLoops));

    // Call the functions
    sch.callFunctions(req);

    // Check it's successful
    auto result = getMpiBatchResult(msg, true);

    // Get the execution graph
    auto execGraph = faabric::util::getFunctionExecGraph(msg);

    // Prepare the expectation
    std::vector<std::string> expectedHostsBefore = { getDistTestMasterIp(),
                                                     getDistTestMasterIp(),
                                                     getDistTestWorkerIp(),
                                                     getDistTestWorkerIp() };
    std::vector<std::string> expectedHostsAfter(4, getDistTestMasterIp());

    // Check expectation against actual execution graph
    checkSchedulingFromExecGraph(
      execGraph, expectedHostsBefore, expectedHostsAfter);
}
*/
}
