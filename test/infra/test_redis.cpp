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

    TEST_CASE("Test redis connection", "[redis]") {
        RedisClient cli;
        cli.connect();

        std::string expected = "foobar";
        std::string actual = cli.check(expected);

        REQUIRE(expected == actual);
    }

    void doEnqueue(RedisClient &cli) {
        cli.connect();
        cli.flushall();

        // Enqueue some values
        cli.enqueue(vars::QUEUE_NAME, vars::VALUE_A);
        cli.enqueue(vars::QUEUE_NAME, vars::VALUE_B);
        cli.enqueue(vars::QUEUE_NAME, vars::VALUE_C);

        // Check expected length
        cli.llen(vars::QUEUE_NAME, [](cpp_redis::reply &r) {
            int64_t actual = r.as_integer();

            REQUIRE(3 == actual);
        });

        cli.sync_commit();
    }

    TEST_CASE("Test blocking enqueue/ dequeue", "[redis]") {
        RedisClient cli;

        // Enqueue
        doEnqueue(cli);

        // Blocking dequeue
        std::vector<std::string> actual = cli.blockingDequeue(vars::QUEUE_NAME);

        REQUIRE(vars::QUEUE_NAME == actual[0]);
        REQUIRE(vars::VALUE_A == actual[1]);

        // Dequeue again
        std::vector<std::string> actual2 = cli.blockingDequeue(vars::QUEUE_NAME);

        REQUIRE(vars::QUEUE_NAME == actual2[0]);
        REQUIRE(vars::VALUE_B == actual2[1]);

        cli.sync_commit();
    }

    TEST_CASE("Test non-blocking enqueue/ dequeue", "[redis]") {
        RedisClient cli;

        // Enqueue
        doEnqueue(cli);

        // Non-blocking dequeue
        cli.dequeue(vars::QUEUE_NAME, [](cpp_redis::reply &reply) {
            std::vector<std::string> keyValue = RedisClient::getKeyValueFromReply(reply);

            REQUIRE(vars::QUEUE_NAME == keyValue[0]);
            REQUIRE(vars::VALUE_A == keyValue[1]);
        });

        cli.dequeue(vars::QUEUE_NAME, [](cpp_redis::reply &reply) {
            std::vector<std::string> keyValue = RedisClient::getKeyValueFromReply(reply);

            REQUIRE(vars::QUEUE_NAME == keyValue[0]);
            REQUIRE(vars::VALUE_B == keyValue[1]);
        });

        cli.sync_commit();
    }

    TEST_CASE("Test redis calling/ retrieving function call", "[redis]") {
        RedisClient cli;
        cli.connect();
        cli.flushall();

        // Request function
        message::FunctionCall call;
        std::string funcName = "my func";
        std::string userName = "some user";
        std::string resultKey = "some result";
        call.set_function(funcName);
        call.set_user(userName);
        call.set_resultkey(resultKey);

        cli.callFunction(call);

        // Check one function call on there
        cli.llen("function_calls", [](cpp_redis::reply &result) {
            REQUIRE(1 == result.as_integer());
        });

        cli.sync_commit();

        // Get the next call
        message::FunctionCall actual = cli.nextFunctionCall();

        REQUIRE(funcName == actual.function());
        REQUIRE(userName == actual.user());
        REQUIRE(resultKey == actual.resultkey());

        cli.sync_commit();
    }

    message::FunctionCall callFunction(RedisClient &cli) {
        cli.connect();
        cli.flushall();

        // Write some function result
        message::FunctionCall call;
        call.set_function("my func");
        call.set_user("some user");
        call.set_resultkey("function 123");

        cli.setFunctionResult(call, true);

        cli.sync_commit();

        // Check result has been written to the right key
        cli.llen("function 123", [](cpp_redis::reply &reply) {
            REQUIRE(1 == reply.as_integer());
        });

        return call;
    }

    TEST_CASE("Test redis reading and writing function results", "[redis]") {
        RedisClient cli;
        message::FunctionCall call = callFunction(cli);

        // Check retrieval method gets the same call out again
        cli.getFunctionResult(call, [](message::FunctionCall &actualCall2) {
            REQUIRE("my func" == actualCall2.function());
            REQUIRE("some user" == actualCall2.user());
            REQUIRE("function 123" == actualCall2.resultkey());
            REQUIRE(actualCall2.success());
        });

        cli.sync_commit();
    }

    TEST_CASE("Test blocking redis reading and writing function results", "[redis]") {
        RedisClient cli;
        message::FunctionCall call = callFunction(cli);

        // Check retrieval method gets the same call out again
        message::FunctionCall actualCall2 = cli.blockingGetFunctionResult(call);
        cli.sync_commit();

        REQUIRE("my func" == actualCall2.function());
        REQUIRE("some user" == actualCall2.user());
        REQUIRE("function 123" == actualCall2.resultkey());
        REQUIRE(actualCall2.success());
    }

}