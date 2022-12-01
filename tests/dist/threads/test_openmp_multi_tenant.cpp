#include <catch2/catch.hpp>
#include "fixtures.h"

#include <unistd.h>

namespace tests {
TEST_CASE_METHOD(DistTestsFixture,
                 "Test two OpenMP functions on the same host",
                 "[threads][openmp]")
{
    // Set enough resources locally
    int nLocalSlots = 10;
    faabric::HostResources res;
    res.set_slots(nLocalSlots);
    sch.setThisHostResources(res);

    // Set up the two messages
    std::shared_ptr<faabric::BatchExecuteRequest> req1 =
      faabric::util::batchExecFactory("omp", "repeated_reduce_sleep", 1);
    std::shared_ptr<faabric::BatchExecuteRequest> req2 =
      faabric::util::batchExecFactory("omp", "repeated_reduce_sleep_2", 1);
    faabric::Message& msg1 = req1->mutable_messages()->at(0);
    msg1.set_cmdline("2 1000");
    faabric::Message& msg2 = req2->mutable_messages()->at(0);
    msg2.set_cmdline("2 1000");

    // Invoke the function
    sch.callFunctions(req1);
    sleep(1);
    sch.callFunctions(req2);

    // Check it's successful
    faabric::Message result1 =
      sch.getFunctionResult(msg1.id(), functionCallTimeout);
    REQUIRE(result1.returnvalue() == 0);
}
}
