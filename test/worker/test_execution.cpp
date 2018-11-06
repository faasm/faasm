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

    TEST_CASE("Test full execution of WASM module", "[worker]") {
        setUp();

        message::FunctionCall call;
        call.set_user("demo");
        call.set_function("echo");
        call.set_inputdata("this is input");
        call.set_resultkey("test_echo");

        // Run the execution
        execFunction(1, call);
        message::FunctionCall result = redis.getFunctionResult(call);

        // Check output
        REQUIRE(result.outputdata() == "this is input");
        REQUIRE(result.success());

        tearDown();
    }

    TEST_CASE("Test executing non-existent function", "[worker]") {
        setUp();

        message::FunctionCall call;
        call.set_user("foobar");
        call.set_function("baz");
        call.set_resultkey("test_invalid");

        execFunction(1, call);
        message::FunctionCall result = redis.getFunctionResult(call);

        REQUIRE(!result.success());
        REQUIRE(result.outputdata() == "foobar - baz is not a valid function");

        tearDown();
    }

    TEST_CASE("Test function chaining", "[worker]") {
        setUp();

        message::FunctionCall call;
        call.set_user("demo");
        call.set_function("chain");
        call.set_resultkey("test_chain");

        // Run the execution
        execFunction(1, call);

        // Check the call executed successfully
        message::FunctionCall result = redis.getFunctionResult(call);
        REQUIRE(result.success());

        // Check the chained calls have been set up
        message::FunctionCall chainA = redis.nextFunctionCall();
        message::FunctionCall chainB = redis.nextFunctionCall();
        message::FunctionCall chainC = redis.nextFunctionCall();

        // Check all are set with the right user
        REQUIRE(chainA.user() == "demo");
        REQUIRE(chainB.user() == "demo");
        REQUIRE(chainC.user() == "demo");

        // Check function names
        REQUIRE(chainA.function() == "echo");
        REQUIRE(chainB.function() == "x2");
        REQUIRE(chainC.function() == "dummy");

        // Check function data
        std::vector<uint8_t> expected0 = {0, 1, 2};
        std::vector<uint8_t> expected1 = {1, 2, 3};
        std::vector<uint8_t> expected2 = {2, 3, 4};
        REQUIRE(util::stringToBytes(chainA.inputdata()) == expected0);
        REQUIRE(util::stringToBytes(chainB.inputdata()) == expected1);
        REQUIRE(util::stringToBytes(chainC.inputdata()) == expected2);

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
        message::FunctionCall call;
        call.set_user("demo");
        call.set_function("state");
        call.set_resultkey("test_state");

        // Execute and check
        execFunction(1, call);
        message::FunctionCall resultA = redis.getFunctionResult(call);
        REQUIRE(resultA.success());

        // Load the state again, it should have a new element
        std::vector<uint8_t> stateA = redis.get(stateKey);
        std::vector<uint8_t> expectedA = {0};
        REQUIRE(stateA == expectedA);

        // Call the function a second time, the state should have another element added
        execFunction(1, call);
        message::FunctionCall resultB = redis.getFunctionResult(call);
        REQUIRE(resultB.success());

        std::vector<uint8_t> stateB = redis.get(stateKey);
        std::vector<uint8_t> expectedB = {0, 1};
        REQUIRE(stateB == expectedB);
    }

    TEST_CASE("Test state increment", "[worker]") {
        setUp();

        // Set up the function call
        message::FunctionCall call;
        call.set_user("demo");
        call.set_function("increment");
        call.set_resultkey("test_state_incr");

        // Execute and check
        execFunction(1, call);
        message::FunctionCall resultA = redis.getFunctionResult(call);
        REQUIRE(resultA.success());
        REQUIRE(resultA.outputdata() == "Counter: 001");

        // Call the function a second time, the state should have been incremented
        execFunction(1, call);
        message::FunctionCall resultB = redis.getFunctionResult(call);
        REQUIRE(resultB.success());
        REQUIRE(resultB.outputdata() == "Counter: 002");
    }
}