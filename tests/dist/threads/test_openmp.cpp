#include <catch2/catch.hpp>

#include "fixtures.h"

#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/batch.h>
#include <faabric/util/string_tools.h>

#define PI_FUNCTION "pi_faasm"

namespace tests {
TEST_CASE_METHOD(DistTestsFixture,
                 "Test OpenMP across hosts",
                 "[threads][openmp]")
{
    conf.overrideCpuCount = 6;

    // TODO(wamr-omp)
    if (faasmConf.wasmVm == "wamr") {
        return;
    }

    // Set this host up to have fewer slots than the number of threads, noting
    // that we take up one local slot with the main thread
    int nLocalSlots = 3;
    faabric::HostResources res;
    res.set_slots(nLocalSlots);
    sch.setThisHostResources(res);

    std::string function;
    SECTION("Not using shared memory") { function = "hellomp"; }

    SECTION("Using shared memory") { function = "omp_checks"; }

    SECTION("Repeated reduce") { function = "repeated_reduce"; }

    SECTION("Pi estimation") { function = PI_FUNCTION; }

    // Set up the message
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("omp", function, 1);
    faabric::Message& msg = req->mutable_messages()->at(0);

    // Invoke the function
    plannerCli.callFunctions(req);

    // Check it's successful
    faabric::Message result =
      plannerCli.getMessageResult(msg, functionCallTimeout);
    REQUIRE(result.returnvalue() == 0);

    // Check one executor used on this host (always the case for threads)
    REQUIRE(sch.getFunctionExecutorCount(msg) == 1);

    // Check specific results
    if (function == PI_FUNCTION) {
        REQUIRE(
          faabric::util::startsWith(result.outputdata(), "Pi estimate: 3.1"));
    }
}
}
