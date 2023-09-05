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
    // Allocate resources so that functions are only invoked in the remote host
    int nMessages = 3;
    setLocalRemoteSlots(0, nMessages, 0, 0);

    // Call a few functions to be executed on the other host
    std::vector<int> msgIds;
    std::string workerIp = getDistTestWorkerIp();
    std::vector<std::string> expectedHosts(nMessages, workerIp);
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("demo", "echo", nMessages);
    int appId = req->messages(0).appid();
    for (int i = 0; i < 3; i++) {
        faabric::Message& msg = req->mutable_messages()->at(i);
        msg.set_inputdata(fmt::format("foobar {}", i));
        msgIds.emplace_back(msg.id());
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
    // Set up this host's resources so that the parent function and one child
    // are in one host, and the other two child functions in another host
    int nLocalSlots = 2;
    setLocalRemoteSlots(nLocalSlots * 2, nLocalSlots, nLocalSlots * 2 - 2, 0);

    // Set up the message
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("demo", "chain", 1);
    faabric::Message& msg = req->mutable_messages()->at(0);
    msg.set_recordexecgraph(true);

    // Call the functions
    plannerCli.callFunctions(req);

    // Check it's successful
    faabric::Message result =
      plannerCli.getMessageResult(msg, functionCallTimeout);
    REQUIRE(result.returnvalue() == 0);

    std::vector<std::string> expectedChainedHosts = { getDistTestMasterIp(),
                                                      getDistTestWorkerIp(),
                                                      getDistTestWorkerIp() };

    // Wait for all chained messages too
    std::vector<std::string> actualChainedHosts;
    for (const int chainedId : faabric::util::getChainedFunctions(msg)) {
        auto chainedResult = plannerCli.getMessageResult(
          msg.appid(), chainedId, functionCallTimeout);
        REQUIRE(chainedResult.returnvalue() == 0);
        actualChainedHosts.push_back(chainedResult.executedhost());
    }

    // Check chained allocation
    REQUIRE(actualChainedHosts == expectedChainedHosts);

    // Check executors on this host
    REQUIRE(sch.getFunctionExecutorCount(msg) == 2);
}
}
