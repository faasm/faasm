#include <catch/catch.hpp>

extern "C" {
#include <emulator/emulator_api.h>
}

#include "utils.h"

#include <util/environment.h>
#include <util/bytes.h>
#include <redis/Redis.h>

#include <worker/WorkerThreadPool.h>
#include <worker/WorkerThread.h>
#include <emulator/emulator.h>

using namespace worker;

namespace tests {
    static std::string originalNsMode;

    static void setUp() {
        cleanSystem();

        message::Message call = util::messageFactory("demo", "chain");
        setEmulatedMessage(call);

        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.clear();
        sch.addNodeToGlobalSet();

        // Network ns requires root
        originalNsMode = util::setEnvVar("NETNS_MODE", "off");
    }

    static void tearDown() {
        util::setEnvVar("NETNS_MODE", originalNsMode);

        cleanSystem();
    }

    TEST_CASE("Test worker initially pre-warmed", "[worker]") {
        setUp();

        WorkerThreadPool pool(1);
        WorkerThread w(1);
        REQUIRE(!w.isBound());
    }

    void checkBound(WorkerThread &w, message::Message &msg, bool isBound) {
        REQUIRE(w.isBound() == isBound);
        if (w.module) {
            REQUIRE(w.module->isBound() == isBound);
        } else {
            REQUIRE(!isBound);
        }
    }

    TEST_CASE("Test binding to function", "[worker]") {
        setUp();

        message::Message call = util::messageFactory("demo", "chain");
        setEmulatedMessage(call);

        WorkerThreadPool pool(1);
        WorkerThread w(1);
        checkBound(w, call, false);

        w.bindToFunction(call);
        checkBound(w, call, true);
    }

    TEST_CASE("Test fixed input with colon", "[worker]") {
        setUp();
        message::Message call = util::messageFactory("demo", "check_input");
        call.set_inputdata("http://www.foobar.com");
        setEmulatedMessage(call);

        execFunction(call);
        tearDown();
    }

    TEST_CASE("Test execution of echo function", "[worker]") {
        setUp();
        message::Message call = util::messageFactory("demo", "echo");
        std::string inputData = "http://www.testinput/foo.com";
        call.set_inputdata(inputData.c_str());
        setEmulatedMessage(call);

        // Run the execution
        const std::string actual = execFunctionWithStringResult(call);
        REQUIRE(actual == inputData);

        tearDown();
    }

    TEST_CASE("Test execution of empty echo function", "[worker]") {
        setUp();
        message::Message call = util::messageFactory("demo", "echo");
        setEmulatedMessage(call);

        // Run the execution
        execFunction(call);

        tearDown();
    }

    TEST_CASE("Test repeat execution of WASM module", "[worker]") {
        setUp();

        message::Message call = util::messageFactory("demo", "echo");
        call.set_inputdata("first input");
        setEmulatedMessage(call);

        // Set up
        WorkerThreadPool pool(1);
        WorkerThread w(1);

        // Bind to function
        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.callFunction(call);
        w.processNextMessage();
        REQUIRE(w.isBound());

        // Run the execution
        w.processNextMessage();

        // Check output from first invocation
        scheduler::GlobalMessageBus &globalBus = scheduler::getGlobalMessageBus();
        message::Message resultA = globalBus.getFunctionResult(call.id(), 1);
        REQUIRE(resultA.outputdata() == "first input");
        REQUIRE(resultA.success());

        // Execute again
        call.set_inputdata("second input");
        call.set_id(0);
        util::setMessageId(call);
        setEmulatedMessage(call);

        sch.callFunction(call);

        w.processNextMessage();

        // Check output from second invocation
        message::Message resultB = globalBus.getFunctionResult(call.id(), 1);
        REQUIRE(resultB.outputdata() == "second input");
        REQUIRE(resultB.success());

        tearDown();
    }

    TEST_CASE("Test bind message causes worker to bind", "[worker]") {
        setUp();

        // Create worker
        WorkerThreadPool pool(1);
        WorkerThread w(1);
        REQUIRE(!w.isBound());

        scheduler::Scheduler &sch = scheduler::getScheduler();

        // Invoke a new call which will require a worker to bind
        message::Message call = util::messageFactory("demo", "echo");
        setEmulatedMessage(call);

        sch.callFunction(call);

        // Check message is on the bind queue
        auto bindQueue = sch.getBindQueue();
        REQUIRE(bindQueue->size() == 1);

        // Process next message
        w.processNextMessage();

        // Check message has been consumed and that worker is now bound
        REQUIRE(w.isBound());
    }

    TEST_CASE("Test function chaining", "[worker]") {
        setUp();

        util::SystemConfig &conf = util::getSystemConfig();
        conf.boundTimeout = 1000;
        conf.unboundTimeout = 1000;

        message::Message call = util::messageFactory("demo", "chain");
        unsigned int messageId = call.id();
        setEmulatedMessage(call);

        // NOTE - for this test to work we have to run multiple threads.
        // TODO - is this necessary? Any way to avoid having threaded tests?

        // Set up a real worker to execute this function. Remove it from the
        // unassigned set and add to handle this function
        WorkerThreadPool pool(4);
        pool.startThreadPool();

        // Make the call
        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.callFunction(call);

        // Await the call executing successfully
        scheduler::GlobalMessageBus &globalBus = scheduler::getGlobalMessageBus();
        message::Message result = globalBus.getFunctionResult(messageId, 1);
        REQUIRE(result.success());

        // Check that we've scaled up to the max functions
        REQUIRE(sch.getFunctionThreadCount(call) == 4);

        // Call again
        message::Message call2 = util::messageFactory("demo", "chain");
        unsigned int messageId2 = call2.id();
        setEmulatedMessage(call2);

        sch.callFunction(call2);

        // Await the call executing successfully
        message::Message result2 = globalBus.getFunctionResult(messageId2, 1);
        REQUIRE(result2.success());

        // Check that we've not scaled up further
        REQUIRE(sch.getFunctionThreadCount(call2) == 4);

        pool.shutdown();

        tearDown();
    }

    void execFuncWithPool(message::Message &call) {
        setUp();

        util::SystemConfig &conf = util::getSystemConfig();
        conf.boundTimeout = 1000;
        conf.unboundTimeout = 1000;

        unsigned int messageId = call.id();
        setEmulatedMessage(call);

        // Set up a real worker to execute this function. Remove it from the
        // unassigned set and add to handle this function
        WorkerThreadPool pool(4);
        pool.startThreadPool();

        // Make the call
        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.callFunction(call);

        // Await the call executing successfully
        scheduler::GlobalMessageBus &globalBus = scheduler::getGlobalMessageBus();
        message::Message result = globalBus.getFunctionResult(messageId, 1);
        REQUIRE(result.success());

        pool.shutdown();

        tearDown();
    }

    TEST_CASE("Test appended state", "[worker]") {
        message::Message call = util::messageFactory("demo", "state_append");
        execFuncWithPool(call);
    }

    TEST_CASE("Test python chaining", "[worker]") {
        message::Message call = util::messageFactory(PYTHON_USER, PYTHON_FUNC);
        call.set_pythonuser("python");
        call.set_pythonfunction("chain");
        call.set_ispython(true);

        execFuncWithPool(call);
    }

    TEST_CASE("Test repeat invocation with state", "[worker]") {
        setUp();

        // Set up the function call
        message::Message call = util::messageFactory("demo", "increment");
        setEmulatedMessage(call);

        // Call function
        WorkerThreadPool pool(1);
        WorkerThread w(1);

        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.callFunction(call);

        // Bind and exec
        w.processNextMessage();
        w.processNextMessage();

        // Check result
        scheduler::GlobalMessageBus &globalBus = scheduler::getGlobalMessageBus();
        message::Message resultA = globalBus.getFunctionResult(call.id(), 1);
        REQUIRE(resultA.success());
        REQUIRE(resultA.outputdata() == "Counter: 001");

        // Call the function a second time, the state should have been incremented
        call.set_id(0);
        util::setMessageId(call);
        setEmulatedMessage(call);

        sch.callFunction(call);
        w.processNextMessage();

        message::Message resultB = globalBus.getFunctionResult(call.id(), 1);
        REQUIRE(resultB.success());
        REQUIRE(resultB.outputdata() == "Counter: 002");
    }

    void checkStateExample(const std::string &funcName, const std::string &keyName, std::vector<uint8_t> expectedOutput,
                           std::vector<uint8_t> expectedRedis) {
        setUp();

        // Set up the function call
        message::Message call = util::messageFactory("demo", funcName);
        setEmulatedMessage(call);

        // Call function
        WorkerThreadPool pool(1);
        WorkerThread w(1);

        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.callFunction(call);

        // Bind and exec
        w.processNextMessage();
        w.processNextMessage();

        // Check result
        scheduler::GlobalMessageBus &globalBus = scheduler::getGlobalMessageBus();
        message::Message result = globalBus.getFunctionResult(call.id(), 1);
        REQUIRE(result.success());
        std::vector<uint8_t> outputBytes = util::stringToBytes(result.outputdata());

        REQUIRE(outputBytes == expectedOutput);

        redis::Redis &redisQueue = redis::Redis::getQueue();
        REQUIRE(redisQueue.get(keyName) == expectedRedis);
    }

    TEST_CASE("Test asynchronous state", "[worker]") {
        checkStateExample(
                "state_async",
                "demo_state_async_example",
                {1, 1, 1, 1},
                {3, 2, 1, 0}
        );
    }

    TEST_CASE("Test offset state", "[worker]") {
        checkStateExample(
                "state_offset",
                "demo_state_offset_example",
                {5, 5, 6, 6, 4},
                {5, 5, 6, 6, 4, 5, 6}
        );
    }

    TEST_CASE("Test memory is reset", "[worker]") {
        cleanSystem();

        message::Message call = util::messageFactory("demo", "heap");
        setEmulatedMessage(call);

        // Call function
        WorkerThreadPool pool(1);
        WorkerThread w(1);

        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.callFunction(call);

        // Process bind
        w.processNextMessage();

        // Check initial pages
        Uptr initialPages = Runtime::getMemoryNumPages(w.module->defaultMemory);

        // Exec the function
        w.processNextMessage();

        // Check page count is equal
        Uptr afterPages = Runtime::getMemoryNumPages(w.module->defaultMemory);
        REQUIRE(afterPages == initialPages);
    }

    void checkCallingFunctionGivesBoolOutput(const std::string &funcName, bool expected) {
        message::Message call = util::messageFactory("demo", funcName);
        setEmulatedMessage(call);

        // Call function
        WorkerThreadPool pool(1);
        WorkerThread w(1);

        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.callFunction(call);

        // Bind and execute
        w.processNextMessage();
        w.processNextMessage();

        // Check output is true
        scheduler::GlobalMessageBus &globalBus = scheduler::getGlobalMessageBus();
        message::Message result = globalBus.getFunctionResult(call.id(), 1);
        REQUIRE(result.success());
        std::vector<uint8_t> outputBytes = util::stringToBytes(result.outputdata());

        std::vector<uint8_t> expectedOutput;

        if (expected) {
            expectedOutput = {1};
        } else {
            expectedOutput = {0};
        }

        REQUIRE(outputBytes == expectedOutput);
    }

    TEST_CASE("Test mmap/munmap", "[worker]") {
        setUp();

        checkCallingFunctionGivesBoolOutput("mmap", true);
    }

    TEST_CASE("Test shared state write pointers", "[worker]") {
        setUp();

        // Run the function to write
        checkCallingFunctionGivesBoolOutput("state_shared_write", true);

        // Run the function to read
        checkCallingFunctionGivesBoolOutput("state_shared_read", true);
    }

    TEST_CASE("Test shared state offset pointers", "[worker]") {
        setUp();

        // Run the function to write
        checkCallingFunctionGivesBoolOutput("state_shared_write_offset", true);

        // Run the function to read
        checkCallingFunctionGivesBoolOutput("state_shared_read_offset", true);
    }

    TEST_CASE("Test pool accounting", "[worker]") {
        cleanSystem();

        WorkerThreadPool pool(5);
        REQUIRE(pool.getThreadCount() == 0);

        message::Message call = util::messageFactory("demo", "noop");
        setEmulatedMessage(call);

        // Add threads and check tokens are taken
        WorkerThread w1(pool.getThreadToken());
        WorkerThread w2(pool.getThreadToken());
        REQUIRE(pool.getThreadCount() == 2);

        // Bind
        w1.bindToFunction(call);
        REQUIRE(pool.getThreadCount() == 2);
    }

    TEST_CASE("Test worker lifecycle interacts with scheduler", "[worker]") {
        cleanSystem();
        redis::Redis &redis = redis::Redis::getQueue();

        WorkerThreadPool pool(5);

        WorkerThread w(1);
        std::string nodeId = util::getNodeId();

        message::Message call = util::messageFactory("demo", "noop");
        setEmulatedMessage(call);

        // Sense check initial scheduler set-up
        scheduler::Scheduler &sch = scheduler::getScheduler();
        auto bindQueue = sch.getBindQueue();
        REQUIRE(sch.getFunctionInFlightCount(call) == 0);
        REQUIRE(sch.getFunctionThreadCount(call) == 0);
        REQUIRE(bindQueue->size() == 0);

        // Call the function
        sch.callFunction(call);

        // Check scheduler set-up
        const std::string warmSetName = sch.getFunctionWarmSetName(call);
        REQUIRE(sch.getFunctionInFlightCount(call) == 1);
        REQUIRE(sch.getFunctionThreadCount(call) == 1);
        REQUIRE(bindQueue->size() == 1);
        REQUIRE(redis.sismember(warmSetName, nodeId));

        // Bind the thread and check it's now registered but in-flight decreased
        w.processNextMessage();
        REQUIRE(w.isBound());
        REQUIRE(sch.getFunctionInFlightCount(call) == 1);
        REQUIRE(sch.getFunctionThreadCount(call) == 1);
        REQUIRE(bindQueue->size() == 0);

        // Execute function and check thread still registered
        w.processNextMessage();
        REQUIRE(sch.getFunctionInFlightCount(call) == 0);
        REQUIRE(sch.getFunctionThreadCount(call) == 1);
        REQUIRE(sch.getFunctionInFlightRatio(call) == 0);
        REQUIRE(bindQueue->size() == 0);

        // Finish thread and check things are reset
        w.finish();
        REQUIRE(sch.getFunctionInFlightCount(call) == 0);
        REQUIRE(sch.getFunctionThreadCount(call) == 0);
        REQUIRE(sch.getFunctionInFlightRatio(call) == 0);
        REQUIRE(bindQueue->size() == 0);
        REQUIRE(!redis.sismember(warmSetName, nodeId));
    }

    TEST_CASE("Test writing file to state", "[worker]") {
        cleanSystem();
        message::Message msg = util::messageFactory("demo", "state_file");
        execFunction(msg);
    }
}