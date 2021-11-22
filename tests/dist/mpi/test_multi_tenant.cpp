#include <catch2/catch.hpp>

#include "fixtures.h"

#include <faabric/scheduler/Scheduler.h>

namespace tests {

// 12/11/2021 - This test deliberately fails as multi-tenant MPI execution is
// still not supported. We keep it here to showcase this is the use case we
// want to support.
TEST_CASE_METHOD(DistTestsFixture,
                 "Test running two MPI functions at the same time",
                 "[.][mpi]")
{
    // Set up this host's resources
    int nLocalSlots = 5;
    faabric::HostResources res;
    res.set_slots(nLocalSlots);
    sch.setThisHostResources(res);

    // Set up the message
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("mpi", "mpi_long_alltoall", 1);
    faabric::Message& msg = req->mutable_messages()->at(0);
    msg.set_ismpi(true);
    msg.set_mpiworldsize(4);

    // Call the functions
    sch.callFunctions(req);

    // Set up the second message
    std::shared_ptr<faabric::BatchExecuteRequest> reqCopy =
      faabric::util::batchExecFactory("mpi", "mpi_long_alltoall", 1);
    faabric::Message& msgCopy = req->mutable_messages()->at(0);
    msgCopy.set_ismpi(true);
    msgCopy.set_mpiworldsize(4);


    // Check both results are successful
    faabric::Message result =
      sch.getFunctionResult(msg.id(), functionCallTimeout);
    REQUIRE(result.returnvalue() == 0);
    faabric::Message resultCopy =
      sch.getFunctionResult(msgCopy.id(), functionCallTimeout);
    REQUIRE(result.returnvalue() == 0);
}
}
