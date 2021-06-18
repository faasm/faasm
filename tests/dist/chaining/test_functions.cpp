#include <catch2/catch.hpp>

#include "fixtures.h"

#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/logging.h>

namespace tests {

TEST_CASE_METHOD(DistTestsFixture,
                 "Test invoking a function on another host",
                 "[scheduler]")
{
    sch.addHostToGlobalSet();

    int nMessages = 3;

    // Remove slots from this host
    int nLocalSlots = 0;
    faabric::HostResources res;
    res.set_slots(nLocalSlots);
    sch.setThisHostResources(res);

    // Call a few functions to be executed on the other host
    std::vector<int> msgIds;
    std::vector<std::string> expectedHosts;
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("demo", "echo", nMessages);
    for (int i = 0; i < 3; i++) {
        faabric::Message& msg = req->mutable_messages()->at(i);
        msg.set_inputdata(fmt::format("foobar {}", i));
        msgIds.emplace_back(msg.id());
        expectedHosts.emplace_back(WORKER_IP);
    }

    // Call the functions
    std::vector<std::string> actualHosts = sch.callFunctions(req);
    REQUIRE(actualHosts == expectedHosts);

    // Check it's successful
    for (int i = 0; i < 3; i++) {
        faabric::Message result = sch.getFunctionResult(msgIds.at(i), 1000);
        REQUIRE(result.returnvalue() == 0);
        REQUIRE(result.outputdata() == fmt::format("foobar {}", i));
        REQUIRE(result.executedhost() == WORKER_IP);
    }
}

TEST_CASE_METHOD(DistTestsFixture, "Test chaining across hosts", "[scheduler]")
{
    sch.addHostToGlobalSet();

    // Set up this host's resources
    int nLocalSlots = 2;
    faabric::HostResources res;
    res.set_slots(nLocalSlots);
    sch.setThisHostResources(res);

    // Set up the message
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("demo", "chain", 1);
    faabric::Message& msg = req->mutable_messages()->at(0);

    // Call the functions
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
