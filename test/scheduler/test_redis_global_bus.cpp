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
        util::SystemConfig &conf = util::getSystemConfig();
        
        // Request function
        message::Message call;
        std::string funcName = "my func";
        std::string userName = "some user";
        std::string inputData = "blahblah";

        call.set_function(funcName);
        call.set_user(userName);
        call.set_inputdata(inputData);
        util::setMessageId(call);

        std::string originalSerialisation = conf.serialisation;

        SECTION("Round trip checks") {
            SECTION("True boolean over JSON") {
                call.set_isasync(true);
                conf.serialisation = "json";
            }

            SECTION("False boolean over JSON") {
                call.set_isasync(false);
                conf.serialisation = "json";
            }

            SECTION("True boolean with protobuf") {
                call.set_isasync(true);
                conf.serialisation = "proto";
            }

            SECTION("False boolean with protobuf") {
                call.set_isasync(false);
                conf.serialisation = "proto";
            }

            // Do round trip
            bus.enqueueMessage(call);
            message::Message actual = bus.nextMessage();

            checkMessageEquality(call, actual);
        }

        SECTION("Check reading/ writing function results") {
            util::setMessageId(call);
            bus.setFunctionResult(call, true);

            // Check result has been written to the right key
            REQUIRE(redis.listLength(call.resultkey()) == 1);

            // Check that some expiry has been set
            long ttl = redis.getTtl(call.resultkey());
            REQUIRE(ttl > 10);

            // Check retrieval method gets the same call out again
            message::Message actualCall2 = bus.getFunctionResult(call.id());

            call.set_success(true);
            checkMessageEquality(call, actualCall2);
        }

        conf.serialisation = originalSerialisation;
    }
}