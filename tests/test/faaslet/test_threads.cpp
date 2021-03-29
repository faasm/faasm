#include "faabric/scheduler/FunctionCallClient.h"
#include "utils.h"
#include <catch2/catch.hpp>

#include <faabric/scheduler/Scheduler.h>
#include <faabric/util/config.h>
#include <faabric/util/func.h>
#include <faabric/util/testing.h>
#include <wavm/WAVMWasmModule.h>

namespace tests {
void checkThreadedFunction(const char* threadFunc, bool runPool)
{
    cleanSystem();

    // Run the function
    faabric::Message msg = faabric::util::messageFactory("demo", threadFunc);

    if (runPool) {
        execFuncWithPool(msg, false, 1, false, 4, false);
    } else {
        execFunction(msg);
    }
}

void runTestLocally(const std::string& func)
{
    cleanSystem();

    // Make sure we have plenty of cores
    int nCores = 10;

    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    faabric::HostResources res;
    res.set_cores(nCores);
    sch.setThisHostResources(res);

    faabric::Message msg = faabric::util::messageFactory("demo", func);
    execFunction(msg);

    REQUIRE(sch.getFunctionInFlightCount(msg) == 0);
}

void runTestDistributed(const std::string& func)
{
    cleanSystem();

    faabric::Message msg = faabric::util::messageFactory("demo", func);
    execFunctionWithRemoteBatch(msg, 4, true);
}

TEST_CASE("Test local-only threading", "[threads]")
{
    runTestLocally("threads_local");
}

TEST_CASE("Run thread checks locally", "[threads]")
{
    runTestLocally("threads_check");
}

TEST_CASE("Run thread checks with chaining", "[threads]")
{
    runTestDistributed("threads_check");
}

TEST_CASE("Run distributed threading check", "[threads]")
{
    runTestDistributed("threads_dist");
}
}
