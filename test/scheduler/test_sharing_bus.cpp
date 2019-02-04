#include <catch/catch.hpp>

#include <scheduler/SharingMessageBus.h>
#include <redis/Redis.h>

using namespace scheduler;
using namespace redis;

namespace tests {
    TEST_CASE("Sharing bus tests", "[scheduler]") {

        SharingMessageBus &bus = SharingMessageBus::getInstance();

        Redis redis = Redis::getQueue();

        std::string thisHost = util::getHostName();
        std::string otherHost = "host_other";
        std::string otherHostQueue = "sharing_host_other";

        std::string inputData = "this is input";

        message::Message msg;
        msg.set_user("user a");
        msg.set_function("func a");
        msg.set_isasync(true);
        msg.set_inputdata(inputData);

        SECTION("Check sharing message with a host puts on relevant queue") {
            bus.shareMessageWithHost(otherHost, msg);

            REQUIRE(redis.listLength(otherHostQueue) == 1);
            const message::Message &actual = bus.nextMessageForHost(otherHost);
            REQUIRE(actual.user() == msg.user());
            REQUIRE(actual.function() == msg.function());
            REQUIRE(actual.isasync() == msg.isasync());
            REQUIRE(actual.inputdata() == msg.inputdata());
        }

    }
}