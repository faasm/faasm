#include <catch2/catch.hpp>

#include "fixtures.h"

#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/batch.h>

namespace tests {

TEST_CASE_METHOD(DistTestsFixture, "Test pthreads across hosts", "[scheduler]")
{
    conf.overrideCpuCount = 6;

    // TODO(wamr-omp)
    if (faasmConf.wasmVm == "wamr") {
        return;
    }

    // Set this host up to ensure the main thread and one child thread execute
    // on this host, but one executes remotely
    int nLocalSlots = 2;
    int nThreads = 3;
    setLocalRemoteSlots(nLocalSlots + 1, nThreads - nLocalSlots, 0, 0);

    std::string user = "demo";
    std::string function = "threads_memory";

    // Set up the message
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory(user, function, 1);
    faabric::Message& msg = req->mutable_messages()->at(0);

    // Invoke the function
    plannerCli.callFunctions(req);

    // Check it's successful
    faabric::Message result = plannerCli.getMessageResult(msg, 10000);
    REQUIRE(result.returnvalue() == 0);

    // Check one executor used on this host (always the case for threads)
    REQUIRE(sch.getFunctionExecutorCount(msg) == 1);
}
}
