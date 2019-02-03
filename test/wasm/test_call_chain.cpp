#include <catch/catch.hpp>
#include "utils.h"

#include <util/bytes.h>
#include <scheduler/Scheduler.h>
#include <wasm/CallChain.h>

using namespace wasm;

namespace tests {

    TEST_CASE("Test executing call chain", "[wasm]") {
        cleanSystem();

        scheduler::Scheduler &sch = scheduler::getScheduler();

        message::Message original;
        original.set_user("demo");
        original.set_function("x2");

        CallChain c(original);

        SECTION("Check executing with no chained calls does nothing") {
            c.execute();

            REQUIRE(sch.getFunctionQueueLength(original) == 0);
        }

        SECTION("Check executing with a chained call adds it to the queue") {
            std::vector<uint8_t> inputData = {0, 1, 2};

            c.addCall("demo", "echo", inputData);
            c.execute();

            message::Message expected;
            expected.set_user("demo");
            expected.set_function("echo");

            REQUIRE(sch.getFunctionQueueLength(expected) == 1);

            message::Message actual = sch.getFunctionQueue(expected)->dequeue();

            REQUIRE(util::stringToBytes(actual.inputdata()) == inputData);
            REQUIRE(!actual.resultkey().empty());
        }
    }
}
