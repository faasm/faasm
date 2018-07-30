#include <catch/catch.hpp>
#include <infra/infra.h>

namespace tests {

    TEST_CASE("Test redis connection", "[redis]") {
        infra::RedisClient cli;
        cli.connect();

        std::string expected = "foobar";
        std::string actual = cli.check(expected);

        cli.sync_commit();
        REQUIRE(expected == actual);
    }

    TEST_CASE("Test redis enqueue/ dequeue", "[redis]") {
        infra::RedisClient cli;
        cli.connect();
        cli.flushall();

        // Enqueue some values
        std::string queueName = "my_queue";
        std::string valueA = "val a";
        std::string valueB = "val b";
        std::string valueC = "val c";

        cli.enqueue("my_queue", valueA);
        cli.enqueue("my_queue", valueB);
        cli.enqueue("my_queue", valueC);

        // Check expected length
        cli.llen(queueName, [](cpp_redis::reply &r) {
            int64_t actual = r.as_integer();

            REQUIRE(3 == actual);
        });

        cli.sync_commit();

        // Now dequeue
        std::vector<std::string> actual = cli.blockingDequeue(queueName);

        REQUIRE("my_queue" == actual[0]);
        REQUIRE(valueA == actual[1]);

        // Dequeue again
        std::vector<std::string> actual2 = cli.blockingDequeue(queueName);

        REQUIRE("my_queue" == actual2[0]);
        REQUIRE(valueB == actual2[1]);

        cli.sync_commit();
    }

    TEST_CASE("Test redis calling/ retrieving function call", "[redis]") {
        infra::RedisClient cli;
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

    TEST_CASE("Test redis reading and writing function results", "[redis]") {
        infra::RedisClient cli;
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

        // Check retrieval method gets the same call out again
        message::FunctionCall actualCall2 = cli.getFunctionResult(call);
        REQUIRE("my func" == actualCall2.function());
        REQUIRE("some user" == actualCall2.user());
        REQUIRE("function 123" == actualCall2.resultkey());
        REQUIRE(actualCall2.success());

        cli.sync_commit();
    }

}