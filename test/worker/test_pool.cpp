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
        Worker w(1);
        w.bindToFunction(call);

        redis.callFunction(call);
        w.runSingle();
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

    TEST_CASE("Test function chaining", "[worker]") {
        setUp();

        message::Message call;
        call.set_user("demo");
        call.set_function("chain");
        call.set_resultkey("test_chain");

        // Set up a real worker to execute this function. Remove it from the
        // unassigned set and add to handle this function
        Worker w(1);
        w.bindToFunction(call);

        // Make the call
        redis.callFunction(call);

        // Execute the worker
        w.runSingle();

        // Check the call executed successfully
        message::Message result = redis.getFunctionResult(call);
        REQUIRE(result.success());

        // Check the chained calls have been set up
        message::Message expectedChainA;
        expectedChainA.set_user("demo");
        expectedChainA.set_user("echo");
        expectedChainA.set_inputdata({0, 1, 2});

        message::Message expectedChainB;
        expectedChainB.set_user("demo");
        expectedChainB.set_user("x2");
        expectedChainB.set_inputdata({1, 2, 3});

        message::Message expectedChainC;
        expectedChainC.set_user("demo");
        expectedChainC.set_user("dummy");
        expectedChainC.set_inputdata({2, 3, 4});

        message::Message chainA = redis.nextMessage(infra::getFunctionQueueName(expectedChainA));
        message::Message chainB = redis.nextMessage(infra::getFunctionQueueName(expectedChainB));
        message::Message chainC = redis.nextMessage(infra::getFunctionQueueName(expectedChainC));

        // Check correct data is set
        REQUIRE(chainA.user() == expectedChainA.user());
        REQUIRE(chainA.function() == expectedChainA.function());
        REQUIRE(chainA.inputdata() == expectedChainA.inputdata());

        REQUIRE(chainB.user() == expectedChainB.user());
        REQUIRE(chainB.function() == expectedChainB.function());
        REQUIRE(chainB.inputdata() == expectedChainB.inputdata());

        REQUIRE(chainC.user() == expectedChainC.user());
        REQUIRE(chainC.function() == expectedChainC.function());
        REQUIRE(chainC.inputdata() == expectedChainC.inputdata());
        
        // Check bind messages also sent
        const message::Message bindMessageA = redis.nextMessage(infra::PREWARM_QUEUE);
        const message::Message bindMessageB = redis.nextMessage(infra::PREWARM_QUEUE);
        const message::Message bindMessageC = redis.nextMessage(infra::PREWARM_QUEUE);

        REQUIRE(bindMessageA.user() == expectedChainA.user());
        REQUIRE(bindMessageA.function() == expectedChainA.function());

        REQUIRE(bindMessageB.user() == expectedChainB.user());
        REQUIRE(bindMessageB.function() == expectedChainB.function());

        REQUIRE(bindMessageC.user() == expectedChainC.user());
        REQUIRE(bindMessageC.function() == expectedChainC.function());

        tearDown();
    }

    TEST_CASE("Test state", "[worker]") {
        setUp();

        // Initially function's state should be an empty array
        // Note, we need to prepend the user to the actual key used in the code
        const char* stateKey = "demo_state_example";
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