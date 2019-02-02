#include <catch/catch.hpp>

#include "utils.h"

#include <scheduler/Scheduler.h>

#include <util/queue.h>

using namespace redis;

namespace tests {
    void checkCallRoundTrip(bool isAsync, bool isSuccess) {
        cleanSystem();

        // Request function
        message::Message call;
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

        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.callFunction(call);

        // Get the next call
        scheduler::InMemoryMessageQueue *queue = sch.getFunctionQueue(call);
        message::Message actual = queue->dequeue();

        REQUIRE(funcName == actual.function());
        REQUIRE(userName == actual.user());
        REQUIRE(inputData == actual.inputdata());
        REQUIRE(isAsync == actual.isasync());
        REQUIRE(isSuccess == actual.success());
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

    TEST_CASE("Test redis reading and writing function results", "[redis]") {
        Redis &redisQueue = Redis::getQueue();
        redisQueue.flushAll();

        // Write some function result
        message::Message call;
        call.set_function("my func");
        call.set_user("some user");
        call.set_resultkey("function 123");


        scheduler::MessageQueue &messageQueue = scheduler::MessageQueue::getGlobalQueue();
        messageQueue.setFunctionResult(call, true);

        // Check result has been written to the right key
        REQUIRE(redisQueue.listLength("function 123") == 1);

        // Check that some expiry has been set
        long ttl = redisQueue.getTtl(call.resultkey());
        REQUIRE(ttl > 10);

        // Check retrieval method gets the same call out again
        message::Message actualCall2 = messageQueue.getFunctionResult(call);

        REQUIRE("my func" == actualCall2.function());
        REQUIRE("some user" == actualCall2.user());
        REQUIRE("function 123" == actualCall2.resultkey());
        REQUIRE(actualCall2.success());
    }

    // TODO - make sure this covers all message queue functions
}