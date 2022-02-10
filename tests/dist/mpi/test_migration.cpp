#include <catch2/catch.hpp>

#include "fixtures.h"

#include <faabric/scheduler/Scheduler.h>

#include <set>

namespace tests {

TEST_CASE_METHOD(DistTestsFixture, "Test migrating an MPI execution", "[mpi]")
{
    // Even though there are enough resources locally, by setting the migration
    // check period in an MPI excution we are effectively forcing to under-
    // provision locally.
    int nLocalSlots = 4;
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
    msg.set_migrationcheckperiod(migrationCheckPeriod);
    msg.set_cmdline(std::to_string(migrationCheckPeriod));

    // Call the functions
    sch.callFunctions(req);

    // Check it's successful
    faabric::Message result =
      sch.getFunctionResult(msg.id(), functionCallTimeout);
    REQUIRE(result.returnvalue() == 0);
}
}
