#include <catch/catch.hpp>

#include <scheduler/SharingMessageBus.h>
#include <redis/Redis.h>

using namespace scheduler;
using namespace redis;

namespace tests {
    TEST_CASE("Sharing bus tests", "[scheduler]") {

        SharingMessageBus &bus = SharingMessageBus::getInstance();

        // TODO add test cases for sharing bus here
    }
}