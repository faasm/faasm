#include <catch/catch.hpp>

#include "utils.h"

#include <scheduler/Scheduler.h>

#include <util/queue.h>

using namespace scheduler;

namespace tests {

    TEST_CASE("Global message queue tests", "[scheduler]") {
        cleanSystem();

        redis::Redis &redis = redis::Redis::getQueue();
        GlobalMessageBus &bus = getGlobalMessageBus();

        // Request function
        message::Message call;
        std::string funcName = "my func";
        std::string userName = "some user";
        std::string inputData = "blahblah";

        call.set_function(funcName);
        call.set_user(userName);
        call.set_inputdata(inputData);

        SECTION("Round trip checks") {
            bool isAsync;
            bool isSuccess;

            SECTION("Synchronous no success") {
                isAsync = false;
                isSuccess = false;
            }

            SECTION("Asynchronous no success") {
                isAsync = true;
                isSuccess = false;
            }

            SECTION("Synchronous success") {
                isAsync = false;
                isSuccess = true;
            }

            SECTION("Asynchronous success") {
                isAsync = true;
                isSuccess = true;
            }

            call.set_isasync(isAsync);
            call.set_success(isSuccess);

            // Do round trip
            bus.enqueueMessage(call);
            message::Message actual = bus.nextMessage();

            REQUIRE(funcName == actual.function());
            REQUIRE(userName == actual.user());
            REQUIRE(inputData == actual.inputdata());
            REQUIRE(isAsync == actual.isasync());
            REQUIRE(isSuccess == actual.success());
        }

        SECTION("Check reading/ writing function results") {
            call.set_resultkey("function 123");
            bus.setFunctionResult(call, true);

            // Check result has been written to the right key
            REQUIRE(redis.listLength("function 123") == 1);

            // Check that some expiry has been set
            long ttl = redis.getTtl(call.resultkey());
            REQUIRE(ttl > 10);

            // Check retrieval method gets the same call out again
            message::Message actualCall2 = bus.getFunctionResult(call);

            REQUIRE("my func" == actualCall2.function());
            REQUIRE("some user" == actualCall2.user());
            REQUIRE("function 123" == actualCall2.resultkey());
            REQUIRE(actualCall2.success());
        }
    }
}