#include <catch/catch.hpp>

#include <worker/worker.h>
#include "utils.h"


using namespace worker;

namespace tests {
    static void setUp() {
        redisQueue.flushAll();
        redisState.flushAll();

        // Network ns requires root
        util::setEnvVar("NETNS_MODE", "off");
    }

    static void tearDown() {
        util::unsetEnvVar("NETNS_MODE");
    }

    void execFunction(message::Message &call) {
        // Set up worker to listen for relevant function
        WorkerThreadPool pool;
        WorkerThread w(pool, 1, 1);
        REQUIRE(w.isInitialised());
        REQUIRE(!w.isBound());

        const std::string expectedPrewarmQueue = infra::Scheduler::getHostPrewarmQueue();

        // Call the function, checking that everything is set up
        infra::Scheduler::callFunction(call);
        REQUIRE(redisQueue.listLength(infra::Scheduler::getFunctionQueueName(call)) == 1);
        REQUIRE(infra::Scheduler::getFunctionCount(call) == 1);
        REQUIRE(redisQueue.listLength(expectedPrewarmQueue) == 1);

        // Process the bind message
        w.processNextMessage();
        REQUIRE(w.isBound());
        REQUIRE(redisQueue.listLength(infra::Scheduler::getFunctionQueueName(call)) == 1);
        REQUIRE(infra::Scheduler::getFunctionCount(call) == 1);
        REQUIRE(redisQueue.listLength(expectedPrewarmQueue) == 0);

        // Now execute the function
        w.processNextMessage();
        REQUIRE(redisQueue.listLength(infra::Scheduler::getFunctionQueueName(call)) == 0);
        REQUIRE(infra::Scheduler::getFunctionCount(call) == 1);
        REQUIRE(redisQueue.listLength(expectedPrewarmQueue) == 0);
    }

    void checkBindMessage(const message::Message &expected) {
        std::string expectedPrewarmQueue = infra::Scheduler::getHostPrewarmQueue();
        const message::Message actual = redisQueue.nextMessage(expectedPrewarmQueue);
        REQUIRE(actual.user() == expected.user());
        REQUIRE(actual.function() == expected.function());
    }

    message::Message checkChainCall(const std::string &user, const std::string &func, const std::string &inputData) {
        message::Message expected;
        expected.set_user(user);
        expected.set_function(func);
        expected.set_inputdata(inputData);

        message::Message actual = redisQueue.nextMessage(infra::Scheduler::getFunctionQueueName(expected));

        REQUIRE(actual.user() == expected.user());
        REQUIRE(actual.function() == expected.function());
        REQUIRE(actual.inputdata() == expected.inputdata());

        return expected;
    }

    TEST_CASE("Test worker initially pre-warmed", "[worker]") {
        setUp();

        WorkerThreadPool pool;
        WorkerThread w(pool, 1, 1);
        REQUIRE(!w.isBound());
        REQUIRE(w.isInitialised());
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

        WorkerThreadPool pool;
        WorkerThread w(pool, 1, 1);
        w.initialise();
        checkBound(w, call, false);

        w.bindToFunction(call);
        checkBound(w, call, true);
    }

    TEST_CASE("Test full execution of WASM module", "[worker]") {
        setUp();

        message::Message call;
        call.set_user("demo");
        call.set_function("echo");
        call.set_inputdata("this is input");
        call.set_resultkey("test_echo");

        // Run the execution
        execFunction(call);
        message::Message result = redisQueue.getFunctionResult(call);

        // Check output
        REQUIRE(result.outputdata() == "this is input");
        REQUIRE(result.success());

        tearDown();
    }

    TEST_CASE("Test bind message causes worker to bind", "[worker]") {
        setUp();

        // Create worker and check it's prewarm
        WorkerThreadPool pool;
        WorkerThread w(pool, 1, 1);
        REQUIRE(!w.isBound());
        REQUIRE(w.isInitialised());

        const std::string expectedPrewarmQueue = infra::Scheduler::getHostPrewarmQueue();

        // Invoke a new call which will require a worker to bind
        message::Message call;
        call.set_user("demo");
        call.set_function("echo");
        infra::Scheduler::callFunction(call);

        // Check message is on the prewarm queue
        REQUIRE(redisQueue.listLength(expectedPrewarmQueue) == 1);

        // Process next message
        w.processNextMessage();

        // Check message has been consumed and that worker is now bound
        REQUIRE(w.isBound());
    }

    TEST_CASE("Test function chaining", "[worker]") {
        setUp();

        message::Message call;
        call.set_user("demo");
        call.set_function("chain");
        call.set_resultkey("test_chain");

        // Set up a real worker to execute this function. Remove it from the
        // unassigned set and add to handle this function
        WorkerThreadPool pool;
        WorkerThread w(pool, 1, 1);
        w.bindToFunction(call);

        // Make the call
        infra::Scheduler::callFunction(call);

        // Execute the worker
        w.processNextMessage();

        // Check the call executed successfully
        message::Message result = redisQueue.getFunctionResult(call);
        REQUIRE(result.success());

        // Check the chained calls have been set up
        message::Message chainA = checkChainCall("demo", "echo", {0, 1, 2});
        message::Message chainB = checkChainCall("demo", "x2", {1, 2, 3});
        message::Message chainC = checkChainCall("demo", "dummy", {2, 3, 4});

        // Check bind messages also sent
        checkBindMessage(call);
        checkBindMessage(chainA);
        checkBindMessage(chainB);
        checkBindMessage(chainC);

        tearDown();
    }

    TEST_CASE("Test state increment", "[worker]") {
        setUp();

        // Set up the function call
        message::Message call;
        call.set_user("demo");
        call.set_function("increment");
        call.set_resultkey("test_state_incr");

        // Call function
        WorkerThreadPool pool;
        WorkerThread w(pool, 1, 1);
        infra::Scheduler::callFunction(call);

        // Bind and exec
        w.processNextMessage();
        w.processNextMessage();

        // Check result
        message::Message resultA = redisQueue.getFunctionResult(call);
        REQUIRE(resultA.success());
        REQUIRE(resultA.outputdata() == "Counter: 001");

        // Call the function a second time, the state should have been incremented
        infra::Scheduler::callFunction(call);
        w.processNextMessage();

        message::Message resultB = redisQueue.getFunctionResult(call);
        REQUIRE(resultB.success());
        REQUIRE(resultB.outputdata() == "Counter: 002");
    }

    void checkStateExample(const std::string &funcName, const std::string &keyName, std::vector<uint8_t> expectedOutput,
                           std::vector<uint8_t> expectedRedis) {
        setUp();

        // Set up the function call
        message::Message call;
        call.set_user("demo");
        call.set_function(funcName);
        call.set_resultkey("check_state_res");

        // Call function
        WorkerThreadPool pool;
        WorkerThread w(pool, 1, 1);
        infra::Scheduler::callFunction(call);

        // Bind and exec
        w.processNextMessage();
        w.processNextMessage();

        // Check result
        message::Message result = redisQueue.getFunctionResult(call);
        REQUIRE(result.success());
        std::vector<uint8_t> outputBytes = util::stringToBytes(result.outputdata());

        REQUIRE(outputBytes == expectedOutput);
        REQUIRE(redisState.get(keyName) == expectedRedis);
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
        message::Message call;
        call.set_user("demo");
        call.set_function("heap");
        call.set_resultkey("test_heap_mem");

        // Call function
        WorkerThreadPool pool;
        WorkerThread w(pool, 1, 1);
        infra::Scheduler::callFunction(call);

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

    void checkCallingFunctionGivesTrueOutput(const std::string &funcName) {
        message::Message call;
        call.set_user("demo");
        call.set_function(funcName);
        call.set_resultkey("test_" + funcName);

        // Call function
        WorkerThreadPool pool;
        WorkerThread w(pool, 1, 1);
        infra::Scheduler::callFunction(call);

        // Bind and execute
        w.processNextMessage();
        w.processNextMessage();

        // Check output is true
        message::Message result = redisQueue.getFunctionResult(call);
        REQUIRE(result.success());
        std::vector<uint8_t> outputBytes = util::stringToBytes(result.outputdata());

        std::vector<uint8_t> expectedOutput = {1};
        REQUIRE(outputBytes == expectedOutput);
    }

    TEST_CASE("Test mmap/munmap", "[worker]") {
        setUp();

        checkCallingFunctionGivesTrueOutput("mmap");
    }

    TEST_CASE("Test shared state pointers", "[worker]") {
        setUp();

        // Run the function to write
        checkCallingFunctionGivesTrueOutput("state_shared_write");

        // Run the function to read
        checkCallingFunctionGivesTrueOutput("state_shared_read");
    }

    TEST_CASE("Test shared state offset pointers", "[worker]") {
        setUp();

        // Run the function to write
        checkCallingFunctionGivesTrueOutput("state_shared_write_offset");

        // Run the function to read
        checkCallingFunctionGivesTrueOutput("state_shared_read_offset");
    }

    TEST_CASE("Test worker pool adds worker hostname to set when starting up", "[worker]") {
        cleanSystem();

        std::string originalHostname = util::getEnvVar("HOSTNAME", "");
        REQUIRE(!originalHostname.empty());

        util::setEnvVar("HOSTNAME", "foo");

        WorkerThreadPool pool;
        REQUIRE(redisQueue.sismember(GLOBAL_WORKER_SET, "foo"));

        redisQueue.srem(GLOBAL_WORKER_SET, "foo");
        util::setEnvVar("HOSTNAME", originalHostname);
        cleanSystem();
    }
}