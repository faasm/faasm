#include <catch/catch.hpp>
#include <infra/infra.h>

using namespace infra;

namespace tests {

    namespace vars {
        const std::string QUEUE_NAME = "my queue";
        const std::string VALUE_A = "val a";
        const std::string VALUE_B = "val b";
        const std::string VALUE_C = "val c";
    }

    void doEnqueue(Redis *cli) {
        cli->flushAll();

        // Enqueue some values
        cli->enqueue(vars::QUEUE_NAME, vars::VALUE_A);
        cli->enqueue(vars::QUEUE_NAME, vars::VALUE_B);
        cli->enqueue(vars::QUEUE_NAME, vars::VALUE_C);

        // Check expected length
        REQUIRE(3 == cli->listLength(vars::QUEUE_NAME));
    }

    TEST_CASE("Test enqueue/ dequeue", "[redis]") {
        Redis cli;

        // Enqueue
        doEnqueue(&cli);

        // Blocking dequeue
        std::string actual = cli.dequeue(vars::QUEUE_NAME);

        REQUIRE(vars::VALUE_A == actual);

        // Dequeue again
        std::string actual2 = cli.dequeue(vars::QUEUE_NAME);

        REQUIRE(vars::VALUE_B == actual2);
    }

    TEST_CASE("Test redis calling/ retrieving function call", "[redis]") {
        Redis cli;
        cli.flushAll();

        // Request function
        message::FunctionCall call;
        std::string funcName = "my func";
        std::string userName = "some user";

        call.set_function(funcName);
        call.set_user(userName);

        // Check resultkey not set initially
        REQUIRE(!call.has_resultkey());

        cli.callFunction(call);

        // Check one function call on there
        REQUIRE(cli.listLength("function_calls") == 1);

        // Get the next call
        message::FunctionCall actual = cli.nextFunctionCall();

        REQUIRE(funcName == actual.function());
        REQUIRE(userName == actual.user());

        // Check result key has now been set
        REQUIRE(actual.has_resultkey());
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