#include <catch2/catch.hpp>

#include "fixtures.h"

#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/batch.h>
#include <faabric/util/logging.h>
#include <faabric/util/string_tools.h>

namespace tests {

TEST_CASE_METHOD(DistTestsFixture,
                 "Test running distributed Pi estimate",
                 "[scheduler]")
{
    // Allocate resources so that ten workers execute locally (+ main message)
    // and ten remotely
    int nWorkers = 20;
    int nLocalSlots = nWorkers / 2;
    setLocalRemoteSlots(2 * nLocalSlots, nLocalSlots, nLocalSlots - 1, 0);

    // Set up the message
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("demo", "pi", 1);
    faabric::Message& msg = req->mutable_messages()->at(0);
    msg.set_inputdata(std::to_string(nWorkers));
    msg.set_recordexecgraph(true);

    // Call the functions
    plannerCli.callFunctions(req);

    std::vector<std::string> actualHosts;
    std::vector<std::string> expectedHosts;

    // Check it's successful
    auto result = plannerCli.getMessageResult(msg, functionCallTimeout);
    REQUIRE(result.returnvalue() == 0);
    actualHosts.push_back(result.executedhost());
    expectedHosts.push_back(getDistTestMasterIp());

    // Wait for all chained messages too
    for (const int chainedId : faabric::util::getChainedFunctions(msg)) {
        auto chainedResult = plannerCli.getMessageResult(
          msg.appid(), chainedId, functionCallTimeout);
        REQUIRE(chainedResult.returnvalue() == 0);
        actualHosts.push_back(chainedResult.executedhost());
        if (expectedHosts.size() <= nLocalSlots) {
            expectedHosts.push_back(getDistTestMasterIp());
        } else {
            expectedHosts.push_back(getDistTestWorkerIp());
        }
    }

    // Get the estimate (check one dp)
    REQUIRE(faabric::util::startsWith(result.outputdata(), "Pi estimate: 3.1"));
    REQUIRE(actualHosts == expectedHosts);
}
}
