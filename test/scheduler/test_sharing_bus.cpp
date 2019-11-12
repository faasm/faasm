#include <catch/catch.hpp>

#include <scheduler/SharingMessageBus.h>
#include <redis/Redis.h>
#include <utils.h>

using namespace scheduler;
using namespace redis;

namespace tests {
    void checkMessageSharingQueueEmpty(const std::string &node) {
        Redis &redis = Redis::getQueue();
        std::string queueName = "sharing_" + node;
        REQUIRE(redis.listLength(queueName) == 0);
    }

    void checkMessageOnSharingQueue(const std::string &node, const message::Message &msg) {
        SharingMessageBus &bus = SharingMessageBus::getInstance();
        Redis &redis = Redis::getQueue();

        std::string queueName = "sharing_" + node;
        REQUIRE(redis.listLength(queueName) == 1);

        const message::Message &actual = bus.nextMessageForNode(node);
        checkMessageEquality(actual, msg);
    }

    TEST_CASE("Sharing bus tests", "[scheduler]") {
        cleanSystem();

        SharingMessageBus &bus = SharingMessageBus::getInstance();
        Scheduler &sch = scheduler::getScheduler();

        std::string thisNode = util::getNodeId();
        std::string otherNodeA = "node_other_a";
        std::string otherNodeB = "node_other_b";

        sch.addNodeToGlobalSet(otherNodeA);
        sch.addNodeToGlobalSet(otherNodeB);

        std::string inputData = "this is input";

        message::Message msg = util::messageFactory("user a", "func a");
        msg.set_isasync(true);
        msg.set_ispython(true);
        msg.set_istypescript(true);
        msg.set_inputdata(inputData);

        SECTION("Check sharing message with a host puts on relevant queue") {
            bus.shareMessageWithNode(otherNodeA, msg);

            checkMessageSharingQueueEmpty(thisNode);
            checkMessageSharingQueueEmpty(otherNodeB);
            checkMessageOnSharingQueue(otherNodeA, msg);
        }

        SECTION("Check broadcast message") {
            bus.broadcastMessage(msg);

            checkMessageOnSharingQueue(thisNode, msg);
            checkMessageOnSharingQueue(otherNodeB, msg);
            checkMessageOnSharingQueue(otherNodeA, msg);
        }
    }

}