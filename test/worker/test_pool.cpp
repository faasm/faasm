#include <catch/catch.hpp>

#include "utils.h"

#include <util/environment.h>
#include <util/bytes.h>
#include <redis/Redis.h>

#include <worker/WorkerThreadPool.h>
#include <worker/WorkerThread.h>
#include <emulator/emulator.h>

using namespace worker;

namespace tests {
    static void setUp() {
        redis::Redis::getState().flushAll();
        redis::Redis::getQueue().flushAll();

        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.clear();
        sch.addNodeToGlobalSet();

        // Network ns requires root
        util::setEnvVar("NETNS_MODE", "off");

        setEmulatorUser("demo");
    }

    static void tearDown() {
        util::unsetEnvVar("NETNS_MODE");

        unsetEmulatorUser();
    }

    void checkBindMessage(const message::Message &expected) {
        scheduler::Scheduler &sch = scheduler::getScheduler();
        const message::Message actual = sch.getBindQueue()->dequeue();

        REQUIRE(actual.user() == expected.user());
        REQUIRE(actual.function() == expected.function());
    }

    message::Message checkChainCall(const std::string &user, const std::string &func, const std::string &inputData) {
        message::Message expected;
        expected.set_user(user);
        expected.set_function(func);
        expected.set_inputdata(inputData);

        scheduler::Scheduler &sch = scheduler::getScheduler();
        message::Message actual = sch.getFunctionQueue(expected)->dequeue();

        REQUIRE(actual.user() == expected.user());
        REQUIRE(actual.function() == expected.function());
        REQUIRE(actual.inputdata() == expected.inputdata());

        return expected;
    }

    TEST_CASE("Test worker initially pre-warmed", "[worker]") {
        setUp();

        WorkerThreadPool pool(1);
        WorkerThread w(1);
        REQUIRE(!w.isBound());
        REQUIRE(w.isInitialised());
    }

    TEST_CASE("Test worker not pre-warmed if not in prewarm mode", "[worker]") {
        setUp();

        util::SystemConfig &conf = util::getSystemConfig();
        conf.prewarm = 0;

        WorkerThreadPool pool(1);

        WorkerThread w(1);

        REQUIRE(!w.isBound());
        REQUIRE(!w.isInitialised());

        conf.prewarm = 1;
    }

    void checkBound(WorkerThread &w, message::Message &msg, bool isBound) {
        REQUIRE(w.isBound() == isBound);
        REQUIRE(w.module->isBound() == isBound);
    }

    TEST_CASE("Test binding to function", "[worker]") {
        setUp();

        message::Message call;
        call.set_user("demo");
        call.set_function("chain");

        WorkerThreadPool pool(1);
        WorkerThread w(1);
        REQUIRE(w.isInitialised());
        checkBound(w, call, false);

        w.bindToFunction(call);
        checkBound(w, call, true);
    }

    TEST_CASE("Test binding to function initialises when in no-prewarm mode", "[worker]") {
        setUp();

        message::Message call;
        call.set_user("demo");
        call.set_function("chain");

        util::SystemConfig &conf = util::getSystemConfig();
        conf.prewarm = 0;

        WorkerThreadPool pool(1);
        WorkerThread w(1);
        REQUIRE(!w.isInitialised());

        w.bindToFunction(call);
        REQUIRE(w.isInitialised());
        checkBound(w, call, true);

        conf.prewarm = 1;
    }

    TEST_CASE("Test execution of echo function", "[worker]") {
        setUp();
        message::Message call = util::messageFactory("demo", "echo");
        call.set_inputdata("this is input");

        // Run the execution
        execFunction(call);
        scheduler::GlobalMessageBus &globalBus = scheduler::getGlobalMessageBus();
        message::Message result = globalBus.getFunctionResult(call.id());

        // Check output
        REQUIRE(result.outputdata() == "this is input");
        REQUIRE(result.success());

        tearDown();
    }

    TEST_CASE("Test execution of empty echo function", "[worker]") {
        setUp();
        message::Message call = util::messageFactory("demo", "echo");

        // Run the execution
        execFunction(call);
        scheduler::GlobalMessageBus &globalBus = scheduler::getGlobalMessageBus();
        message::Message result = globalBus.getFunctionResult(call.id());

        // Check output
        REQUIRE(result.outputdata() == "Nothing to echo");
        REQUIRE(result.success());

        tearDown();
    }

    TEST_CASE("Test repeat execution of WASM module", "[worker]") {
        setUp();

        message::Message call = util::messageFactory("demo", "echo");
        call.set_inputdata("first input");

        // Set up
        WorkerThreadPool pool(1);
        WorkerThread w(1);

        // Bind to function
        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.callFunction(call);
        w.processNextMessage();
        REQUIRE(w.isBound());

        // Snapshot the memory to allow restore
        const char *snapshotKey = "repeatTestEmpty";
        w.module->snapshotFullMemory(snapshotKey);

        // Run the execution
        w.processNextMessage();

        // Check output from first invocation
        scheduler::GlobalMessageBus &globalBus = scheduler::getGlobalMessageBus();
        message::Message resultA = globalBus.getFunctionResult(call.id());
        REQUIRE(resultA.outputdata() == "first input");
        REQUIRE(resultA.success());

        // Restore memory
        w.module->restoreFullMemory(snapshotKey);
        
        // Execute again
        call.set_inputdata("second input");
        util::setMessageId(call);

        sch.callFunction(call);

        w.processNextMessage();

        // Check output from second invocation
        message::Message resultB = globalBus.getFunctionResult(call.id());
        REQUIRE(resultB.outputdata() == "second input");
        REQUIRE(resultB.success());

        tearDown();
    }

    TEST_CASE("Test bind message causes worker to bind", "[worker]") {
        setUp();

        // Create worker and check it's prewarm
        WorkerThreadPool pool(1);
        WorkerThread w(1);
        REQUIRE(!w.isBound());
        REQUIRE(w.isInitialised());

        scheduler::Scheduler &sch = scheduler::getScheduler();

        // Invoke a new call which will require a worker to bind
        message::Message call = util::messageFactory("demo", "echo");
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
        int messageId = call.id();

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
        message::Message result = globalBus.getFunctionResult(messageId);
        REQUIRE(result.success());

        pool.shutdown();

        tearDown();
    }

    TEST_CASE("Test repeat invocation with state", "[worker]") {
        setUp();

        // Set up the function call
        message::Message call = util::messageFactory("demo", "increment");

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
        message::Message resultA = globalBus.getFunctionResult(call.id());
        REQUIRE(resultA.success());
        REQUIRE(resultA.outputdata() == "Counter: 001");

        // Call the function a second time, the state should have been incremented
        util::setMessageId(call);
        sch.callFunction(call);
        w.processNextMessage();

        message::Message resultB = globalBus.getFunctionResult(call.id());
        REQUIRE(resultB.success());
        REQUIRE(resultB.outputdata() == "Counter: 002");
    }

    void checkStateExample(const std::string &funcName, const std::string &keyName, std::vector<uint8_t> expectedOutput,
                           std::vector<uint8_t> expectedRedis) {
        setUp();

        // Set up the function call
        message::Message call = util::messageFactory("demo", funcName);

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
        message::Message result = globalBus.getFunctionResult(call.id());
        REQUIRE(result.success());
        std::vector<uint8_t> outputBytes = util::stringToBytes(result.outputdata());

        REQUIRE(outputBytes == expectedOutput);

        redis::Redis &redisQueue = redis::Redis::getQueue();
        REQUIRE(redisQueue.get(keyName) == expectedRedis);
    }

    TEST_CASE("Test synchronous state", "[worker]") {
        checkStateExample(
                "state_sync",
                "demo_state_example",
                {0, 1, 2, 3},
                {0, 1, 2, 3}
        );
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
        message::Message result = globalBus.getFunctionResult(call.id());
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

    TEST_CASE("Test config", "[worker]") {
        setUp();

        bool expected;
        SECTION("With full async true") {
            util::setEnvVar("FULL_ASYNC", "1");
            util::getSystemConfig().reset();
            expected = true;
        }

        SECTION("With full async false") {
            util::setEnvVar("FULL_ASYNC", "0");
            util::getSystemConfig().reset();
            expected = false;
        }

        checkCallingFunctionGivesBoolOutput("read_conf", expected);
        util::unsetEnvVar("FULL_ASYNC");
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

        message::Message call;
        call.set_user("demo");
        call.set_function("noop");

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

        // Sense check initial scheduler set-up
        scheduler::Scheduler &sch = scheduler::getScheduler();
        auto bindQueue = sch.getBindQueue();
        REQUIRE(sch.getFunctionQueueLength(call) == 0);
        REQUIRE(sch.getFunctionThreadCount(call) == 0);
        REQUIRE(bindQueue->size() == 0);

        // Call the function
        sch.callFunction(call);

        // Check scheduler set-up
        const std::string workerSetName = sch.getFunctionWarmSetName(call);
        REQUIRE(sch.getFunctionQueueLength(call) == 1);
        REQUIRE(sch.getFunctionThreadCount(call) == 1);
        REQUIRE(bindQueue->size() == 1);
        REQUIRE(redis.sismember(workerSetName, nodeId));

        // Bind the thread and check it's now registered
        w.processNextMessage();
        REQUIRE(w.isBound());
        REQUIRE(sch.getFunctionQueueLength(call) == 1);
        REQUIRE(sch.getFunctionThreadCount(call) == 1);
        REQUIRE(bindQueue->size() == 0);

        // Execute function and check thread still registered
        w.processNextMessage();
        REQUIRE(sch.getFunctionQueueLength(call) == 0);
        REQUIRE(sch.getFunctionThreadCount(call) == 1);
        REQUIRE(bindQueue->size() == 0);

        // Finish thread and check things are reset
        w.finish();
        REQUIRE(sch.getFunctionQueueLength(call) == 0);
        REQUIRE(sch.getFunctionThreadCount(call) == 0);
        REQUIRE(bindQueue->size() == 0);
        REQUIRE(!redis.sismember(workerSetName, nodeId));
    }

    TEST_CASE("Test argv", "[worker]") {
        cleanSystem();

        message::Message msg = util::messageFactory("demo", "argv");
        execFunction(msg);
    }
}