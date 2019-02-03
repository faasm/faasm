#include <catch/catch.hpp>

#include "utils.h"
#include <edge/FunctionEndpoint.h>
#include <scheduler/GlobalMessageQueue.h>

using namespace Pistache;

namespace tests {
    static void setUp() {
        redis::Redis::getQueue().flushAll();
    }

    TEST_CASE("Test invoking a function", "[edge]") {
        cleanSystem();

        // Note - must be async to avoid needing a result
        message::Message call;
        call.set_isasync(true);
        call.set_user("demo");
        call.set_function("echo");
        call.set_inputdata("abc");

        // Get global queue
        scheduler::Scheduler &sch = scheduler::getScheduler();
        scheduler::GlobalMessageQueue globalQueue = sch.getGlobalQueue();

        edge::FunctionEndpoint endpoint;
        endpoint.handleFunction(call);

        const message::Message actual = globalQueue.nextMessage();

        REQUIRE(actual.user() == "demo");
        REQUIRE(actual.function() == "echo");
        REQUIRE(actual.inputdata() == "abc");
    }

    TEST_CASE("Test invoking a non-existent function", "[worker]") {
        setUp();

        // Note - must be async to avoid needing a result
        message::Message call;
        call.set_user("foobar");
        call.set_function("baz");

        edge::FunctionEndpoint endpoint;
        std::string msg = endpoint.handleFunction(call);

        REQUIRE(msg == "foobar/baz is not a valid function");
    }
}