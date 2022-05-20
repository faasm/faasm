#include <catch2/catch.hpp>

#include "fixtures.h"

#include <faabric/scheduler/Scheduler.h>

#include <set>

namespace tests {

TEST_CASE_METHOD(DistTestsFixture, "Test migrating an MPI execution", "[mpi]")
{
    // Under-allocate resources
    int nLocalSlots = 2;
    int mpiWorldSize = 4;
    // Try to migrate at 50% of execution
    int migrationCheckPeriod = 5;
    faabric::HostResources res;
    res.set_slots(nLocalSlots);
    sch.setThisHostResources(res);

    // Set up the message
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("mpi", "migrate", 1);
    faabric::Message& msg = req->mutable_messages()->at(0);
    msg.set_ismpi(true);
    msg.set_mpiworldsize(mpiWorldSize);
    msg.set_recordexecgraph(true);
    // Set a low migration check period to detect the mgiration right away
    msg.set_migrationcheckperiod(migrationCheckPeriod);
    int numLoops = 10000;
    msg.set_cmdline(fmt::format("{} {}", migrationCheckPeriod, numLoops));

    // Call the functions
    sch.callFunctions(req);

    // Sleep for a while to let the scheduler schedule the MPI calls, and then
    // update the local slots so that a migration opportunity appears
    SLEEP_MS(500);
    res.set_slots(mpiWorldSize);
    sch.setThisHostResources(res);

    // Check it's successful
    faabric::Message result =
      sch.getFunctionResult(msg.id(), functionCallTimeout);
    REQUIRE(result.returnvalue() == 0);
}
}
