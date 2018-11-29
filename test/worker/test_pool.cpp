#include <catch/catch.hpp>

#include <worker/worker.h>

using namespace worker;

namespace tests {
    static infra::Redis redis;

    void setUp() {
        redis.flushAll();

        // Network ns requires root
        util::setEnvVar("NETNS_MODE", "off");
    }

    void tearDown() {
        util::unsetEnvVar("NETNS_MODE");
    }

    void execFunction(message::Message &call) {
        // Set up worker to listen for relevant function
        WorkerThread w(1);
        w.bindToFunction(call);

        redis.callFunction(call);
        w.runSingle();
    }

    void checkBindMessage(const message::Message &expected) {
        const message::Message actual = redis.nextMessage(infra::PREWARM_QUEUE);
        REQUIRE(actual.user() == expected.user());
        REQUIRE(actual.function() == expected.function());
    }

    message::Message checkChainCall(const std::string &user, const std::string &func, const std::string &inputData) {
        message::Message expected;
        expected.set_user(user);
        expected.set_function(func);
        expected.set_inputdata(inputData);

        message::Message actual = redis.nextMessage(infra::getFunctionQueueName(expected));

        REQUIRE(actual.user() == expected.user());
        REQUIRE(actual.function() == expected.function());
        REQUIRE(actual.inputdata() == expected.inputdata());

        return expected;
    }

    TEST_CASE("Test worker initially pre-warmed", "[worker]") {
        setUp();

        REQUIRE(redis.getCounter(infra::PREWARM_COUNTER) == 0);
        REQUIRE(redis.getCounter(infra::COLD_COUNTER) == 0);

        WorkerThread w(2);
        REQUIRE(!w.isBound());
        REQUIRE(w.isInitialised());

        REQUIRE(redis.getCounter(infra::PREWARM_COUNTER) == 1);
        REQUIRE(redis.getCounter(infra::COLD_COUNTER) == 0);
    }

    void checkBound(WorkerThread &w, message::Message &msg, bool isBound) {
        REQUIRE(w.isBound() == isBound);
        REQUIRE(w.module->isBound() == isBound);
    }

    TEST_CASE("Test worker doesn't initialise by default when enough workers in system", "[worker]") {
        setUp();

        REQUIRE(redis.getCounter(infra::PREWARM_COUNTER) == 0);
        REQUIRE(redis.getCounter(infra::COLD_COUNTER) == 0);

        // Set up enough fake workers to meet our prewarm target
        util::SystemConfig conf = util::getSystemConfig();
        int nWorkers = conf.prewarm_target;
        for (int i = 0; i < nWorkers; i++) {
            redis.incr(infra::PREWARM_COUNTER);
        }

        WorkerThread w(1);
        REQUIRE(!w.isBound());
        REQUIRE(!w.isInitialised());
        REQUIRE(redis.getCounter(infra::PREWARM_COUNTER) == nWorkers);
        REQUIRE(redis.getCounter(infra::COLD_COUNTER) == 1);
    }

    TEST_CASE("Test binding to function", "[worker]") {
        setUp();

        message::Message call;
        call.set_user("demo");
        call.set_function("chain");

        WorkerThread w(1);
        w.initialise();
        checkBound(w, call, false);
        w.bindToFunction(call);
        checkBound(w, call, true);

        // Check that binding another worker does nothing as the target has already been reached
        WorkerThread w2(2);
        checkBound(w2, call, false);
        w2.bindToFunction(call);

        checkBound(w2, call, false);
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
        message::Message result = redis.getFunctionResult(call);

        // Check output
        REQUIRE(result.outputdata() == "this is input");
        REQUIRE(result.success());

        tearDown();
    }

    TEST_CASE("Test bind message causes worker to bind", "[worker]") {
        setUp();

        // Check prewarm empty to begin with
        REQUIRE(redis.getCounter(infra::PREWARM_COUNTER) == 0);
        REQUIRE(redis.getCounter(infra::COLD_COUNTER) == 0);

        // Create worker and check it's in prewarm set
        WorkerThread w(2);
        REQUIRE(!w.isBound());
        REQUIRE(w.isInitialised());
        REQUIRE(redis.getCounter(infra::PREWARM_COUNTER) == 1);
        REQUIRE(redis.getCounter(infra::COLD_COUNTER) == 0);

        // Request a worker to bind
        message::Message call;
        call.set_user("demo");
        call.set_function("echo");
        message::Message bindMessage = infra::buildBindMessage(call);
        redis.enqueueMessage(infra::PREWARM_QUEUE, bindMessage);

        // Check message is on the prewarm queue
        REQUIRE(redis.listLength(infra::PREWARM_QUEUE) == 1);

        // Process next message
        w.runSingle();

        // Check message has been consumed and that worker is now bound
        REQUIRE(w.isBound());
        REQUIRE(redis.listLength(infra::PREWARM_QUEUE) == 0);
        REQUIRE(redis.getCounter(infra::PREWARM_COUNTER) == 0);
        REQUIRE(redis.getCounter(infra::COLD_COUNTER) == 0);

        // Check that the corresponding pre-warm message has been added to the cold queue
        REQUIRE(redis.listLength(infra::COLD_QUEUE) == 1);
        const message::Message actual = redis.nextMessage(infra::COLD_QUEUE);
        REQUIRE(actual.type() == message::Message_MessageType_PREWARM);
    }

    TEST_CASE("Test function chaining", "[worker]") {
        setUp();

        message::Message call;
        call.set_user("demo");
        call.set_function("chain");
        call.set_resultkey("test_chain");

        // Set up a real worker to execute this function. Remove it from the
        // unassigned set and add to handle this function
        WorkerThread w(1);
        w.bindToFunction(call);

        // Make the call
        redis.callFunction(call);

        // Execute the worker
        w.runSingle();

        // Check the call executed successfully
        message::Message result = redis.getFunctionResult(call);
        REQUIRE(result.success());

        // Check the chained calls have been set up
        message::Message chainA = checkChainCall("demo", "echo", {0, 1, 2});
        message::Message chainB = checkChainCall("demo", "x2", {1, 2, 3});
        message::Message chainC = checkChainCall("demo", "dummy", {2, 3, 4});

        // Check bind messages also sent
        checkBindMessage(chainA);
        checkBindMessage(chainB);
        checkBindMessage(chainC);

        tearDown();
    }

    TEST_CASE("Test state", "[worker]") {
        setUp();

        // Initially function's state should be an empty array
        // Note, we need to prepend the user to the actual key used in the code
        const char *stateKey = "demo_state_example";
        std::vector<uint8_t> initialState = redis.get(stateKey);
        REQUIRE(initialState.empty());

        // Set up the function call
        message::Message call;
        call.set_user("demo");
        call.set_function("state");
        call.set_resultkey("test_state");

        // Execute and check
        execFunction(call);
        message::Message resultA = redis.getFunctionResult(call);
        REQUIRE(resultA.success());

        // Load the state again, it should have a new element
        std::vector<uint8_t> stateA = redis.get(stateKey);
        std::vector<uint8_t> expectedA = {0};
        REQUIRE(stateA == expectedA);

        // Call the function a second time, the state should have another element added
        execFunction(call);
        message::Message resultB = redis.getFunctionResult(call);
        REQUIRE(resultB.success());

        std::vector<uint8_t> stateB = redis.get(stateKey);
        std::vector<uint8_t> expectedB = {0, 1};
        REQUIRE(stateB == expectedB);
    }

    TEST_CASE("Test state increment", "[worker]") {
        setUp();

        // Set up the function call
        message::Message call;
        call.set_user("demo");
        call.set_function("increment");
        call.set_resultkey("test_state_incr");

        // Execute and check
        execFunction(call);
        message::Message resultA = redis.getFunctionResult(call);
        REQUIRE(resultA.success());
        REQUIRE(resultA.outputdata() == "Counter: 001");

        // Call the function a second time, the state should have been incremented
        execFunction(call);
        message::Message resultB = redis.getFunctionResult(call);
        REQUIRE(resultB.success());
        REQUIRE(resultB.outputdata() == "Counter: 002");
    }
}