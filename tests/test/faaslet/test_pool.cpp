#include <catch2/catch.hpp>

#include "utils.h"

#include <faabric/redis/Redis.h>
#include <faabric/util/environment.h>

#include <faaslet/FaasletPool.h>
#include <wavm/WAVMWasmModule.h>

using namespace faaslet;
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

TEST_CASE("Test worker initially pre-warmed", "[faaslet]")
{
    setUp();

    FaasletPool pool(1);
    Faaslet w(1);
    REQUIRE(!w.isBound());
}

void checkBound(Faaslet& w, faabric::Message& msg, bool isBound)
{
    REQUIRE(w.isBound() == isBound);
    if (w.module) {
        REQUIRE(w.module->isBound() == isBound);
    } else {
        REQUIRE(!isBound);
    }
}

TEST_CASE("Test binding to function", "[faaslet]")
{
    setUp();

    faabric::Message call = faabric::util::messageFactory("demo", "chain");

    FaasletPool pool(1);
    Faaslet w(1);
    checkBound(w, call, false);

    w.bindToFunction(call);
    checkBound(w, call, true);
}

TEST_CASE("Test binding twice causes error unless forced", "[faaslet]")
{
    setUp();

    faabric::Message callA = faabric::util::messageFactory("demo", "chain");

    FaasletPool pool(1);
    Faaslet w(1);

    // Bind once
    w.bindToFunction(callA);

    // Binding twice should throw
    REQUIRE_THROWS(w.bindToFunction(callA));

    // Forcing second bind should be ok
    w.bindToFunction(callA, true);

    // Forcing bind to another function should fail
    faabric::Message callB = faabric::util::messageFactory("demo", "echo");
    REQUIRE_THROWS(w.bindToFunction(callB, true));
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

    faabric::Message call = faabric::util::messageFactory("demo", "echo");
    call.set_inputdata("first input");

    // Set up
    FaasletPool pool(1);
    Faaslet w(1);

    // Bind to function
    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    sch.callFunction(call);
    w.processNextMessage();
    REQUIRE(w.isBound());

    // Run the execution
    w.processNextMessage();

    // Check output from first invocation
    faabric::Message resultA = sch.getFunctionResult(call.id(), 1);
    REQUIRE(resultA.outputdata() == "first input");
    REQUIRE(resultA.returnvalue() == 0);

    // Execute again
    call.set_inputdata("second input");
    call.set_id(0);
    faabric::util::setMessageId(call);

    sch.callFunction(call);

    w.processNextMessage();

    // Check output from second invocation
    faabric::Message resultB = sch.getFunctionResult(call.id(), 1);
    REQUIRE(resultB.outputdata() == "second input");
    REQUIRE(resultB.returnvalue() == 0);

    tearDown();
}

TEST_CASE("Test bind message causes worker to bind", "[faaslet]")
{
    setUp();

    // Create worker
    FaasletPool pool(1);
    Faaslet w(1);
    REQUIRE(!w.isBound());

    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();

    // Invoke a new call which will require a worker to bind
    faabric::Message call = faabric::util::messageFactory("demo", "echo");

    sch.callFunction(call);

    // Check message is on the bind queue
    auto bindQueue = sch.getBindQueue();
    REQUIRE(bindQueue->size() == 1);

    // Process next message
    w.processNextMessage();

    // Check message has been consumed and that worker is now bound
    REQUIRE(w.isBound());
}

TEST_CASE("Test memory is reset", "[faaslet]")
{
    cleanSystem();

    faabric::Message call = faabric::util::messageFactory("demo", "heap");

    // Call function
    Faaslet w(1);

    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    sch.callFunction(call);

    // Process bind
    w.processNextMessage();

    // Check initial pages
    wasm::WAVMWasmModule* modulePtr =
      static_cast<wasm::WAVMWasmModule*>(w.module.get());
    Uptr initialPages = Runtime::getMemoryNumPages(modulePtr->defaultMemory);

    // Exec the function
    w.processNextMessage();

    // Check page count is equal
    Uptr afterPages = Runtime::getMemoryNumPages(modulePtr->defaultMemory);
    REQUIRE(afterPages == initialPages);
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

TEST_CASE("Test pool accounting", "[faaslet]")
{
    cleanSystem();

    FaasletPool pool(5);
    REQUIRE(pool.getThreadCount() == 0);

    faabric::Message call = faabric::util::messageFactory("demo", "noop");

    // Add threads and check tokens are taken
    Faaslet w1(pool.getThreadToken());
    Faaslet w2(pool.getThreadToken());
    REQUIRE(pool.getThreadCount() == 2);

    // Bind
    w1.bindToFunction(call);
    REQUIRE(pool.getThreadCount() == 2);
}

TEST_CASE("Test worker lifecycle interacts with scheduler", "[faaslet]")
{
    cleanSystem();
    faabric::redis::Redis& redis = faabric::redis::Redis::getQueue();

    FaasletPool pool(5);

    Faaslet w(1);
    std::string thisHost = faabric::util::getSystemConfig().endpointHost;

    faabric::Message call = faabric::util::messageFactory("demo", "noop");

    // Sense check initial scheduler set-up
    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    auto bindQueue = sch.getBindQueue();
    REQUIRE(sch.getFunctionInFlightCount(call) == 0);
    REQUIRE(sch.getFunctionWarmNodeCount(call) == 0);
    REQUIRE(bindQueue->size() == 0);

    // Call the function
    sch.callFunction(call);

    // Check scheduler set-up
    const std::string warmSetName = sch.getFunctionWarmSetName(call);
    REQUIRE(sch.getFunctionInFlightCount(call) == 1);
    REQUIRE(sch.getFunctionWarmNodeCount(call) == 1);
    REQUIRE(bindQueue->size() == 1);
    REQUIRE(redis.sismember(warmSetName, thisHost));

    // Bind the thread and check it's now registered but in-flight decreased
    w.processNextMessage();
    REQUIRE(w.isBound());
    REQUIRE(sch.getFunctionInFlightCount(call) == 1);
    REQUIRE(sch.getFunctionWarmNodeCount(call) == 1);
    REQUIRE(bindQueue->size() == 0);

    // Execute function and check thread still registered
    w.processNextMessage();
    REQUIRE(sch.getFunctionInFlightCount(call) == 0);
    REQUIRE(sch.getFunctionWarmNodeCount(call) == 1);
    REQUIRE(sch.getFunctionInFlightRatio(call) == 0);
    REQUIRE(bindQueue->size() == 0);

    // Finish thread and check things are reset
    w.finish();
    REQUIRE(sch.getFunctionInFlightCount(call) == 0);
    REQUIRE(sch.getFunctionWarmNodeCount(call) == 0);
    REQUIRE(sch.getFunctionInFlightRatio(call) == 0);
    REQUIRE(bindQueue->size() == 0);
    REQUIRE(!redis.sismember(warmSetName, thisHost));
}
}
