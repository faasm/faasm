#include "utils.h"
#include <catch2/catch.hpp>

#include <faabric/scheduler/FunctionCallClient.h>
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

    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();

    faabric::Message msg = faabric::util::messageFactory("demo", func);
    execFunction(msg);

    // Check the system does its accounting
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
