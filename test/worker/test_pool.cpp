#include <catch/catch.hpp>

#include <worker/worker.h>
#include "utils.h"


using namespace worker;

namespace tests {
    static void setUp() {
        infra::Redis::getState().flushAll();
        infra::Redis::getQueue().flushAll();

        // Network ns requires root
        util::setEnvVar("NETNS_MODE", "off");
    }

    static void tearDown() {
        util::unsetEnvVar("NETNS_MODE");
    }

    WorkerThread execFunction(message::Message &call) {
        infra::Redis &redisQueue = infra::Redis::getQueue();

        // Set up worker to listen for relevant function
        WorkerThreadPool pool(1, 1);
        WorkerThread w(pool, 1, 1);
        REQUIRE(w.isInitialised());
        REQUIRE(!w.isBound());

        infra::Scheduler &sch = infra::getScheduler();
        const std::string expectedPrewarmQueue = sch.getHostPrewarmQueue();

        // Call the function, checking that everything is set up
        sch.callFunction(call);
        REQUIRE(redisQueue.listLength(sch.getFunctionQueueName(call)) == 1);
        REQUIRE(sch.getFunctionCount(call) == 1);
        REQUIRE(redisQueue.listLength(expectedPrewarmQueue) == 1);

        // Process the bind message
        w.processNextMessage();
        REQUIRE(w.isBound());
        REQUIRE(redisQueue.listLength(sch.getFunctionQueueName(call)) == 1);
        REQUIRE(sch.getFunctionCount(call) == 1);
        REQUIRE(redisQueue.listLength(expectedPrewarmQueue) == 0);

        // Now execute the function
        w.processNextMessage();
        REQUIRE(redisQueue.listLength(sch.getFunctionQueueName(call)) == 0);
        REQUIRE(sch.getFunctionCount(call) == 1);
        REQUIRE(redisQueue.listLength(expectedPrewarmQueue) == 0);

        return w;
    }

    void checkBindMessage(const message::Message &expected) {
        infra::Redis &redisQueue = infra::Redis::getQueue();

        infra::Scheduler &sch = infra::getScheduler();
        std::string expectedPrewarmQueue = sch.getHostPrewarmQueue();
        const message::Message actual = redisQueue.nextMessage(expectedPrewarmQueue);
        REQUIRE(actual.user() == expected.user());
        REQUIRE(actual.function() == expected.function());
    }

    message::Message checkChainCall(const std::string &user, const std::string &func, const std::string &inputData) {
        infra::Redis &redisQueue = infra::Redis::getQueue();

        message::Message expected;
        expected.set_user(user);
        expected.set_function(func);
        expected.set_inputdata(inputData);

        infra::Scheduler &sch = infra::getScheduler();
        message::Message actual = redisQueue.nextMessage(sch.getFunctionQueueName(expected));

        REQUIRE(actual.user() == expected.user());
        REQUIRE(actual.function() == expected.function());
        REQUIRE(actual.inputdata() == expected.inputdata());

        return expected;
    }

    TEST_CASE("Test worker initially pre-warmed", "[worker]") {
        setUp();

        WorkerThreadPool pool(1, 1);
        WorkerThread w(pool, 1, 1);
        REQUIRE(!w.isBound());
        REQUIRE(w.isInitialised());
    }

    TEST_CASE("Test worker not pre-warmed if no prewarm token given", "[worker]") {
        setUp();

        WorkerThreadPool pool(1, 1);

        // Note prewarm token -1 here
        WorkerThread w(pool, 1, -1);

        REQUIRE(!w.isBound());
        REQUIRE(!w.isInitialised());
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

        WorkerThreadPool pool(1, 1);
        WorkerThread w(pool, 1, 1);
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

        WorkerThreadPool pool(1, 1);
        WorkerThread w(pool, 1, -1);
        REQUIRE(!w.isInitialised());

        w.bindToFunction(call);
        REQUIRE(w.isInitialised());
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
        infra::Redis &redisQueue = infra::Redis::getQueue();
        message::Message result = redisQueue.getFunctionResult(call);

        // Check output
        REQUIRE(result.outputdata() == "this is input");
        REQUIRE(result.success());

        tearDown();
    }

    TEST_CASE("Test repeat execution of WASM module", "[worker]") {
        setUp();
        infra::Redis &redisQueue = infra::Redis::getQueue();

        message::Message call;
        call.set_user("demo");
        call.set_function("echo");
        call.set_inputdata("first input");
        call.set_resultkey("test_repeat_a");

        // Run the execution
        WorkerThread w = execFunction(call);

        // Check output from first invocation
        message::Message resultA = redisQueue.getFunctionResult(call);
        REQUIRE(resultA.outputdata() == "first input");
        REQUIRE(resultA.success());

        // Execute again
        call.set_inputdata("second input");
        call.set_resultkey("test_repeat_b");

        infra::Scheduler &sch = infra::getScheduler();
        sch.callFunction(call);

        w.processNextMessage();

        // Check output from second invocation
        message::Message resultB = redisQueue.getFunctionResult(call);
        REQUIRE(resultB.outputdata() == "second input");
        REQUIRE(resultB.success());

        tearDown();
    }

    TEST_CASE("Test bind message causes worker to bind", "[worker]") {
        setUp();

        // Create worker and check it's prewarm
        WorkerThreadPool pool(1, 1);
        WorkerThread w(pool, 1, 1);
        REQUIRE(!w.isBound());
        REQUIRE(w.isInitialised());

        infra::Scheduler &sch = infra::getScheduler();
        const std::string expectedPrewarmQueue = sch.getHostPrewarmQueue();

        // Invoke a new call which will require a worker to bind
        message::Message call;
        call.set_user("demo");
        call.set_function("echo");
        sch.callFunction(call);

        // Check message is on the prewarm queue
        infra::Redis &redisQueue = infra::Redis::getQueue();
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
        WorkerThreadPool pool(1, 1);
        WorkerThread w(pool, 1, 1);
        w.bindToFunction(call);

        // Make the call
        infra::Scheduler &sch = infra::getScheduler();
        sch.callFunction(call);

        // Execute the worker
        w.processNextMessage();

        // Check the call executed successfully
        infra::Redis &redisQueue = infra::Redis::getQueue();
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

    TEST_CASE("Test repeat invocation with state", "[worker]") {
        setUp();

        // Set up the function call
        message::Message call;
        call.set_user("demo");
        call.set_function("increment");
        call.set_resultkey("test_state_incr");

        // Call function
        WorkerThreadPool pool(1, 1);
        WorkerThread w(pool, 1, 1);

        infra::Scheduler &sch = infra::getScheduler();
        sch.callFunction(call);

        // Bind and exec
        w.processNextMessage();
        w.processNextMessage();

        // Check result
        infra::Redis &redisQueue = infra::Redis::getQueue();
        message::Message resultA = redisQueue.getFunctionResult(call);
        REQUIRE(resultA.success());
        REQUIRE(resultA.outputdata() == "Counter: 001");

        // Call the function a second time, the state should have been incremented
        sch.callFunction(call);
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
        WorkerThreadPool pool(1, 1);
        WorkerThread w(pool, 1, 1);

        infra::Scheduler &sch = infra::getScheduler();
        sch.callFunction(call);

        // Bind and exec
        w.processNextMessage();
        w.processNextMessage();

        // Check result
        infra::Redis &redisQueue = infra::Redis::getQueue();
        message::Message result = redisQueue.getFunctionResult(call);
        REQUIRE(result.success());
        std::vector<uint8_t> outputBytes = util::stringToBytes(result.outputdata());

        REQUIRE(outputBytes == expectedOutput);
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

        message::Message call;
        call.set_user("demo");
        call.set_function("heap");
        call.set_resultkey("test_heap_mem");

        // Call function
        WorkerThreadPool pool(1, 1);
        WorkerThread w(pool, 1, 1);

        infra::Scheduler &sch = infra::getScheduler();
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

    void checkCallingFunctionGivesTrueOutput(const std::string &funcName) {
        message::Message call;
        call.set_user("demo");
        call.set_function(funcName);
        call.set_resultkey("test_" + funcName);

        // Call function
        WorkerThreadPool pool(1, 1);
        WorkerThread w(pool, 1, 1);

        infra::Scheduler &sch = infra::getScheduler();
        sch.callFunction(call);

        // Bind and execute
        w.processNextMessage();
        w.processNextMessage();

        // Check output is true
        infra::Redis &redisQueue = infra::Redis::getQueue();
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

        WorkerThreadPool pool(1, 1);
        infra::Redis &redisQueue = infra::Redis::getQueue();
        REQUIRE(redisQueue.sismember(GLOBAL_WORKER_SET, "foo"));

        redisQueue.srem(GLOBAL_WORKER_SET, "foo");
        util::setEnvVar("HOSTNAME", originalHostname);
        cleanSystem();
    }

    TEST_CASE("Test finishing thread releases prewarm and thread tokens when not bound", "[worker]") {
        cleanSystem();

        WorkerThreadPool pool(10, 5);
        REQUIRE(pool.getPrewarmCount() == 0);
        REQUIRE(pool.getThreadCount() == 0);

        // Add threads and check tokens are taken
        WorkerThread w1(pool, pool.getThreadToken(), pool.getPrewarmToken());
        WorkerThread w2(pool, pool.getThreadToken(), pool.getPrewarmToken());
        REQUIRE(pool.getPrewarmCount() == 2);
        REQUIRE(pool.getThreadCount() == 2);

        // Remove one and check tokens returned
        w1.finish();
        REQUIRE(pool.getPrewarmCount() == 1);
        REQUIRE(pool.getThreadCount() == 1);
    }

    TEST_CASE("Test thread releases prewarm token when bound then thread token when finishing", "[worker]") {
        cleanSystem();

        WorkerThreadPool pool(10, 5);
        REQUIRE(pool.getPrewarmCount() == 0);
        REQUIRE(pool.getThreadCount() == 0);

        message::Message call;
        call.set_user("demo");
        call.set_function("noop");

        // Add threads and check tokens are taken
        WorkerThread w1(pool, pool.getThreadToken(), pool.getPrewarmToken());
        WorkerThread w2(pool, pool.getThreadToken(), pool.getPrewarmToken());
        REQUIRE(pool.getPrewarmCount() == 2);
        REQUIRE(pool.getThreadCount() == 2);

        // Bind and check prewarm token returned
        w1.bindToFunction(call);
        REQUIRE(pool.getPrewarmCount() == 1);
        REQUIRE(pool.getThreadCount() == 2);

        // Finish and check thread token also returned
        w1.finish();
        REQUIRE(pool.getPrewarmCount() == 1);
        REQUIRE(pool.getThreadCount() == 1);
    }

    TEST_CASE("Test worker lifecycle interacts with scheduler", "[worker]") {
        cleanSystem();
        infra::Redis &redis = infra::Redis::getQueue();

        WorkerThreadPool pool(5, 5);

        WorkerThread w(pool, 1, 1);
        std::string hostname = util::getHostName();

        message::Message call;
        call.set_user("demo");
        call.set_function("noop");

        // Sense check initial scheduler set-up
        infra::Scheduler &sch = infra::getScheduler();
        REQUIRE(sch.getFunctionQueueLength(call) == 0);
        REQUIRE(sch.getFunctionCount(call) == 0);
        REQUIRE(sch.getLocalThreadCount(call) == 0);

        // Call the function
        sch.callFunction(call);

        // Check scheduler set-up
        const std::string workerSetName = sch.getFunctionWorkerSetName(call);
        REQUIRE(sch.getFunctionQueueLength(call) == 1);
        REQUIRE(sch.getFunctionCount(call) == 1);
        REQUIRE(redis.sismember(workerSetName, hostname));

        // Check that thread isn't currently registered locally
        REQUIRE(sch.getLocalThreadCount(call) == 0);

        // Bind the thread and check it's now registered
        w.processNextMessage();
        REQUIRE(sch.getLocalThreadCount(call) == 1);

        // Execute function and check thread still registered
        w.processNextMessage();
        REQUIRE(sch.getLocalThreadCount(call) == 1);

        // Finish thread and check things are reset
        w.finish();
        REQUIRE(sch.getFunctionQueueLength(call) == 0);
        REQUIRE(sch.getFunctionCount(call) == 0);
        REQUIRE(sch.getLocalThreadCount(call) == 0);
        REQUIRE(!redis.sismember(workerSetName, hostname));
    }
}