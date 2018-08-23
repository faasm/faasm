#include <catch/catch.hpp>
#include <infra/infra.h>
#include <util/util.h>

using namespace infra;

namespace tests {

    namespace vars {
        const std::string QUEUE_NAME = "my queue";

        const std::string VALUE_A = "val a";
        const std::string VALUE_B = "val b";
        const std::string VALUE_C = "val c";

        const std::vector<uint8_t> BYTES_A = util::stringToBytes(VALUE_A);
        const std::vector<uint8_t> BYTES_B = util::stringToBytes(VALUE_B);
        const std::vector<uint8_t> BYTES_C = util::stringToBytes(VALUE_C);
    }

    void doEnqueue(Redis *cli) {
        cli->flushAll();

        // Enqueue some values
        std::vector<uint8_t> bytesA = util::stringToBytes(vars::VALUE_A);
        cli->enqueue(vars::QUEUE_NAME, vars::BYTES_A);
        cli->enqueue(vars::QUEUE_NAME, vars::BYTES_B);
        cli->enqueue(vars::QUEUE_NAME, vars::BYTES_C);

        // Check expected length
        REQUIRE(3 == cli->listLength(vars::QUEUE_NAME));
    }

    TEST_CASE("Test enqueue/ dequeue", "[redis]") {
        Redis cli;

        // Enqueue
        doEnqueue(&cli);

        // Blocking dequeue
        std::vector<uint8_t> actual = cli.dequeue(vars::QUEUE_NAME);

        REQUIRE(vars::BYTES_A == actual);

        // Dequeue again
        std::vector<uint8_t> actual2 = cli.dequeue(vars::QUEUE_NAME);

        REQUIRE(vars::BYTES_B == actual2);
    }

    void checkCallRoundTrip(bool isAsync, bool isSuccess) {
        Redis cli;
        cli.flushAll();

        // Request function
        message::FunctionCall call;
        std::string funcName = "my func";
        std::string userName = "some user";
        std::string inputData = "blahblah";

        call.set_function(funcName);
        call.set_user(userName);
        call.set_inputdata(inputData);
        call.set_isasync(isAsync);
        call.set_success(isSuccess);

        // Check resultkey not set initially
        REQUIRE(!call.has_resultkey());

        cli.callFunction(call);

        // Check one function call on there
        REQUIRE(cli.listLength("function_calls") == 1);

        // Get the next call
        message::FunctionCall actual = cli.nextFunctionCall();

        REQUIRE(funcName == actual.function());
        REQUIRE(userName == actual.user());
        REQUIRE(inputData == actual.inputdata());
        REQUIRE(isAsync == actual.isasync());
        REQUIRE(isSuccess == actual.success());

        // Check result key has now been set
        REQUIRE(actual.has_resultkey());
    }

    TEST_CASE("Test redis round trip sync fail", "[redis]") {
        checkCallRoundTrip(false, false);
    }

    TEST_CASE("Test redis round trip sync success", "[redis]") {
        checkCallRoundTrip(false, true);
    }

    TEST_CASE("Test redis round trip async fail", "[redis]") {
        checkCallRoundTrip(true, false);
    }

    TEST_CASE("Test redis round trip async success", "[redis]") {
        checkCallRoundTrip(true, true);
    }

    message::FunctionCall callFunction(Redis *cli) {
        cli->flushAll();

        // Write some function result
        message::FunctionCall call;
        call.set_function("my func");
        call.set_user("some user");
        call.set_resultkey("function 123");

        cli->setFunctionResult(call, true);

        // Check result has been written to the right key
        REQUIRE(cli->listLength("function 123") == 1);

        return call;
    }

    TEST_CASE("Test redis reading and writing function results", "[redis]") {
        Redis cli;
        message::FunctionCall call = callFunction(&cli);

        // Check retrieval method gets the same call out again
        message::FunctionCall actualCall2 = cli.getFunctionResult(call);

        REQUIRE("my func" == actualCall2.function());
        REQUIRE("some user" == actualCall2.user());
        REQUIRE("function 123" == actualCall2.resultkey());
        REQUIRE(actualCall2.success());
    }

}