#include <catch2/catch.hpp>

#include "fixtures.h"

#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/batch.h>
#include <faabric/util/logging.h>

namespace tests {

TEST_CASE_METHOD(DistTestsFixture,
                 "Test invoking a function on another host",
                 "[scheduler]")
{
    int nMessages = 3;

    // Remove slots from this host
    int nLocalSlots = 0;
    faabric::HostResources res;
    res.set_slots(nLocalSlots);
    sch.setThisHostResources(res);

    // Call a few functions to be executed on the other host
    std::vector<int> msgIds;
    std::vector<std::string> expectedHosts;
    std::string workerIp = getDistTestWorkerIp();
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("demo", "echo", nMessages);
    int appId = req->messages(0).appid();
    for (int i = 0; i < 3; i++) {
        faabric::Message& msg = req->mutable_messages()->at(i);
        msg.set_inputdata(fmt::format("foobar {}", i));
        msgIds.emplace_back(msg.id());
        expectedHosts.emplace_back(workerIp);
    }

    // Call the functions
    std::vector<std::string> actualHosts = plannerCli.callFunctions(req).hosts;
    REQUIRE(actualHosts == expectedHosts);

    // Check it's successful
    for (int i = 0; i < 3; i++) {
        faabric::Message result =
          plannerCli.getMessageResult(appId, msgIds.at(i), functionCallTimeout);
        REQUIRE(result.returnvalue() == 0);
        REQUIRE(result.outputdata() == fmt::format("foobar {}", i));
        REQUIRE(result.executedhost() == workerIp);
    }
}

TEST_CASE_METHOD(DistTestsFixture, "Test chaining across hosts", "[scheduler]")
{
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
    plannerCli.callFunctions(req);

    // Check it's successful
    faabric::Message result =
      plannerCli.getMessageResult(msg, functionCallTimeout);
    REQUIRE(result.returnvalue() == 0);

    // Check executors on this host
    REQUIRE(sch.getFunctionExecutorCount(msg) == 2);
}
}
