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
    // The chain function will spawn three other functions, and one of the
    // three functions will spawn yet a fourth one. We schedule three functions
    // locally, and two remotely
    int nLocalSlots = 3;
    setLocalRemoteSlots(2 * nLocalSlots, nLocalSlots, nLocalSlots, 0);

    // Set up the message
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("demo", "chain", 1);
    faabric::Message& msg = req->mutable_messages()->at(0);
    msg.set_recordexecgraph(true);

    // Call the functions
    // PROBLEM: it may happen that the chained function finishes before the
    // next one has been chained
    plannerCli.callFunctions(req);

    // Check it's successful
    faabric::Message result =
      plannerCli.getMessageResult(msg, functionCallTimeout);
    REQUIRE(result.returnvalue() == 0);

    std::vector<std::string> expectedChainedHosts = { getDistTestMasterIp(),
                                                      getDistTestMasterIp(),
                                                      getDistTestWorkerIp() };

    // Wait for all chained messages too
    std::vector<std::string> actualChainedHosts;
    auto chainedIds = faabric::util::getChainedFunctions(msg);
    std::vector<int> nestedChainedIds;
    for (int chainedId : chainedIds) {
        auto chainedResult = plannerCli.getMessageResult(
          msg.appid(), chainedId, functionCallTimeout);
        REQUIRE(chainedResult.returnvalue() == 0);
        actualChainedHosts.push_back(chainedResult.executedhost());

        // See if the chained function did any nested chains
        for (int nestedChainedId :
             faabric::util::getChainedFunctions(chainedResult)) {
            nestedChainedIds.push_back(nestedChainedId);
        }
    }

    // Wait for all nested chained messages
    for (const int nestedChainedId : nestedChainedIds) {
        auto nestedResult = plannerCli.getMessageResult(
          msg.appid(), nestedChainedId, functionCallTimeout);
        REQUIRE(nestedResult.returnvalue() == 0);
    }

    // Check chained allocation
    REQUIRE(actualChainedHosts == expectedChainedHosts);

    // Check executors on this host
    REQUIRE(sch.getFunctionExecutorCount(msg) == 3);
}
}
