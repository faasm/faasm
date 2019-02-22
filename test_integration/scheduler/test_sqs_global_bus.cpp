#include <catch/catch.hpp>

#include <aws/SQSWrapper.h>
#include <util/func.h>
#include <util/config.h>
#include <scheduler/Scheduler.h>
#include <scheduler/AWSMessageBus.h>

using namespace awswrapper;

namespace tests {
    TEST_CASE("SQS message bus integration test", "[aws]") {
        scheduler::GlobalMessageBus &bus = scheduler::getGlobalMessageBus();
        util::SystemConfig &conf = util::getSystemConfig();
        
        SECTION("Check global bus of correct type") {
            // Dynamic cast to null ptr means not of that type
            auto castPtr = dynamic_cast<scheduler::AWSMessageBus *>(&bus);
            REQUIRE(castPtr != nullptr);
        }

        SECTION("Round trip checks") {
            message::Message msg;
            std::string funcName = "my func";
            std::string userName = "some user";
            std::string inputData = "blahblah";

            msg.set_function(funcName);
            msg.set_user(userName);
            msg.set_inputdata(inputData);
            msg.set_isasync(true);

            message::Message actual;

            SECTION("Check message with JSON encoding") {
                conf.serialisation = "json";
                bus.enqueueMessage(msg);
                actual = bus.nextMessage();
            }

            SECTION("Check message with protobuf objects") {
                conf.serialisation = "proto";
                bus.enqueueMessage(msg);
                actual = bus.nextMessage();
            }

            REQUIRE(actual.user() == msg.user());
            REQUIRE(actual.function() == msg.function());
            REQUIRE(actual.inputdata() == msg.inputdata());
            REQUIRE(actual.isasync() == msg.isasync());
        }
    }
}