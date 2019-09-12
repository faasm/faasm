#include <catch/catch.hpp>

#include <scheduler/SharingMessageBus.h>
#include <redis/Redis.h>
#include <utils.h>

using namespace scheduler;
using namespace redis;

namespace tests {
    TEST_CASE("Sharing bus tests", "[scheduler]") {
        SharingMessageBus &bus = SharingMessageBus::getInstance();

        Redis &redis = Redis::getQueue();

        std::string thisNode = util::getNodeId();
        std::string otherNode = "node_other";
        std::string otherNodeQueue = "sharing_node_other";

        std::string inputData = "this is input";

        message::Message msg;
        msg.set_user("user a");
        msg.set_function("func a");
        msg.set_isasync(true);
        msg.set_ispython(true);
        msg.set_istypescript(true);
        msg.set_inputdata(inputData);

        SECTION("Check sharing message with a host puts on relevant queue") {
            bus.shareMessageWithNode(otherNode, msg);

            REQUIRE(redis.listLength(otherNodeQueue) == 1);
            const message::Message &actual = bus.nextMessageForNode(otherNode);

            checkMessageEquality(actual, msg);
        }

    }
}