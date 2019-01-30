#include <catch/catch.hpp>

#include "utils.h"

#include <scheduler/scheduler.h>

using namespace scheduler;
using namespace util;

namespace tests {
    TEST_CASE("Test listening/ stopping listening on queue map", "[scheduler]") {
        LocalQueueMap &queueMap = LocalQueueMap::getInstance();
        queueMap.clear();

        message::Message msg;
        msg.set_user("user a");
        msg.set_function("func a");

        REQUIRE(queueMap.getFunctionQueueLength(msg) == 0);
        REQUIRE(queueMap.getFunctionThreadCount(msg) == 0);

        // Listen to the queue for this function
        queueMap.listenToQueue(msg);

        REQUIRE(queueMap.getFunctionQueueLength(msg) == 0);
        REQUIRE(queueMap.getFunctionThreadCount(msg) == 1);

        // Stop listening
        queueMap.stopListeningToQueue(msg);

        REQUIRE(queueMap.getFunctionQueueLength(msg) == 0);
        REQUIRE(queueMap.getFunctionThreadCount(msg) == 0);
    }

    TEST_CASE("Test enqueueing messages") {
        LocalQueueMap &queueMap = LocalQueueMap::getInstance();
        queueMap.clear();

        message::Message bindMsg;
        bindMsg.set_user("user a");
        bindMsg.set_function("func a");
        bindMsg.set_type(message::Message_MessageType_BIND);

        message::Message callMsg;
        callMsg.set_user("user a");
        callMsg.set_function("func a");
        callMsg.set_type(message::Message_MessageType_CALL);

        queueMap.enqueueMessage(bindMsg);
        queueMap.enqueueMessage(callMsg);

        InMemoryMessageQueue *funcQueue = queueMap.getFunctionQueue(callMsg);
        InMemoryMessageQueue *bindQueue = queueMap.getBindQueue();

        REQUIRE(funcQueue->size() == 1);
        REQUIRE(bindQueue->size() == 1);

        message::Message actualCall = funcQueue->dequeue();
        message::Message actualBind = bindQueue->dequeue();

        REQUIRE(actualCall.type() == message::Message_MessageType_CALL);
        REQUIRE(actualBind.type() == message::Message_MessageType_BIND);
    }
}
