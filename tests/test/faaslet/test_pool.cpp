#include <catch2/catch.hpp>

#include "utils.h"

#include <faabric/redis/Redis.h>
#include <faabric/runner/FaabricMain.h>
#include <faabric/util/environment.h>

#include <faaslet/Faaslet.h>
#include <wavm/WAVMWasmModule.h>

using namespace WAVM;

namespace tests {
static std::string originalNsMode;

static void setUp()
{
    cleanSystem();

    faabric::Message call = faabric::util::messageFactory("demo", "chain");

    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    sch.shutdown();
    sch.addHostToGlobalSet();

    // Network ns requires root
    originalNsMode = faabric::util::setEnvVar("NETNS_MODE", "off");
}

static void tearDown()
{
    faabric::util::setEnvVar("NETNS_MODE", originalNsMode);
    cleanSystem();
}

TEST_CASE("Test execution of empty echo function", "[faaslet]")
{
    setUp();
    faabric::Message call = faabric::util::messageFactory("demo", "echo");

    // Run the execution
    execFunction(call);

    tearDown();
}

TEST_CASE("Test repeat execution of WASM module", "[faaslet]")
{
    setUp();

    auto fac = std::make_shared<faaslet::FaasletFactory>();
    faabric::runner::FaabricMain m(fac);
    m.startRunner();

    faabric::Message call = faabric::util::messageFactory("demo", "echo");
    call.set_inputdata("first input");

    // Bind to function
    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    sch.callFunction(call);

    // Check output from first invocation
    faabric::Message resultA = sch.getFunctionResult(call.id(), 1);
    REQUIRE(resultA.outputdata() == "first input");
    REQUIRE(resultA.returnvalue() == 0);

    // Execute again
    call.set_inputdata("second input");
    call.set_id(0);
    faabric::util::setMessageId(call);

    sch.callFunction(call);

    // Check output from second invocation
    faabric::Message resultB = sch.getFunctionResult(call.id(), 1);
    REQUIRE(resultB.outputdata() == "second input");
    REQUIRE(resultB.returnvalue() == 0);

    m.shutdown();
    tearDown();
}

TEST_CASE("Test mmap/munmap", "[faaslet]")
{
    setUp();

    checkCallingFunctionGivesBoolOutput("demo", "mmap", true);
}

TEST_CASE("Test big mmap", "[faaslet]")
{
    setUp();
    faabric::Message msg = faabric::util::messageFactory("demo", "mmap_big");
    execFunction(msg);
}
}
