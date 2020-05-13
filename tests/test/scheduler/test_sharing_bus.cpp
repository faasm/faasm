#include <catch/catch.hpp>

#include <scheduler/SharingMessageBus.h>
#include <redis/Redis.h>
#include <utils.h>

using namespace scheduler;
using namespace redis;

namespace tests {
    void checkMessageSharingQueueEmpty(const std::string &host) {
        Redis &redis = Redis::getQueue();
        std::string queueName = "sharing_" + host;
        REQUIRE(redis.listLength(queueName) == 0);
    }

    void checkMessageOnSharingQueue(const std::string &host, const message::Message &msg) {
        SharingMessageBus &bus = SharingMessageBus::getInstance();
        Redis &redis = Redis::getQueue();

        std::string queueName = "sharing_" + host;
        REQUIRE(redis.listLength(queueName) == 1);

        const message::Message &actual = bus.nextMessageForHost(host);
        checkMessageEquality(actual, msg);
    }

    TEST_CASE("Sharing bus tests", "[scheduler]") {
        cleanSystem();

        SharingMessageBus &bus = SharingMessageBus::getInstance();
        Scheduler &sch = scheduler::getScheduler();

        std::string thisHost = util::getSystemConfig().endpointHost;
        std::string otherHostA = "host_other_a";
        std::string otherHostB = "host_other_b";

        sch.addHostToGlobalSet(otherHostA);
        sch.addHostToGlobalSet(otherHostB);

        std::string inputData = "this is input";

        message::Message msg = util::messageFactory("user a", "func a");
        msg.set_isasync(true);
        msg.set_ispython(true);
        msg.set_istypescript(true);
        msg.set_inputdata(inputData);

        SECTION("Check sharing message with a host puts on relevant queue") {
            bus.shareMessageWithHost(otherHostA, msg);

            checkMessageSharingQueueEmpty(thisHost);
            checkMessageSharingQueueEmpty(otherHostB);
            checkMessageOnSharingQueue(otherHostA, msg);
        }

        SECTION("Check broadcast message") {
            bus.broadcastMessage(msg);

            checkMessageOnSharingQueue(thisHost, msg);
            checkMessageOnSharingQueue(otherHostB, msg);
            checkMessageOnSharingQueue(otherHostA, msg);
        }
    }

}