#include <catch2/catch.hpp>

#include "fixtures.h"

#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/string_tools.h>
#include <faabric/util/logging.h>

namespace tests {

TEST_CASE_METHOD(DistTestsFixture,
                 "Test running distributed Pi estimate",
                 "[scheduler]")
{
    int nWorkers = 20;

    // Make sure only half can be executed on this host
    int nLocalSlots = 10;
    faabric::HostResources res;
    res.set_slots(nLocalSlots);
    sch.setThisHostResources(res);

    // Set up the message
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("demo", "pi", 1);
    faabric::Message& msg = req->mutable_messages()->at(0);
    msg.set_inputdata(std::to_string(nWorkers));

    // Call the functions
    sch.callFunctions(req);

    // Check it's successful
    faabric::Message result =
      sch.getFunctionResult(msg.id(), functionCallTimeout);
    REQUIRE(result.returnvalue() == 0);

    // Get the estimate (check one dp)
    std::string outputData = msg.outputdata();
    REQUIRE(faabric::util::startsWith("Pi estimate: 3.1", outputData));
}
}
