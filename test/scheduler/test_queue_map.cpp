#include <catch/catch.hpp>

#include "utils.h"

#include <scheduler/scheduler.h>

using namespace scheduler;

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
}
