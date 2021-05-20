#include <catch2/catch.hpp>

#include "fixtures.h"

#include <faabric/scheduler/Scheduler.h>

namespace tests {
TEST_CASE_METHOD(DistTestsFixture, "Test threading across hosts", "[scheduler]")
{
    auto& sch = faabric::scheduler::getScheduler();
    sch.addHostToGlobalSet();

    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    conf.overrideCpuCount = 6;

    // Set this host up to have fewer slots than the number of threads
    int nLocalSlots = 2;
    faabric::HostResources res;
    res.set_slots(nLocalSlots);
    sch.setThisHostResources(res);

    // Set up the message
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("omp", "hellomp", 1);
    faabric::Message& msg = req->mutable_messages()->at(0);

    // Invoke it
    sch.callFunctions(req);

    // Check it's successful
    faabric::Message result = sch.getFunctionResult(msg.id(), 1000);
    REQUIRE(result.returnvalue() == 0);

    // Check executors on this host
    REQUIRE(sch.getFunctionExecutorCount(msg) == 2);

    // Check other host is registered
    std::set<std::string> expectedRegisteredHosts = { WORKER_IP };
    REQUIRE(sch.getFunctionRegisteredHosts(msg) == expectedRegisteredHosts);
}
}
