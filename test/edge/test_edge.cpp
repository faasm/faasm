#include <catch/catch.hpp>

#include "utils.h"
#include <edge/FunctionEndpoint.h>
#include <scheduler/GlobalMessageBus.h>

using namespace Pistache;

namespace tests {
    TEST_CASE("Test invoking a function", "[edge]") {
        cleanSystem();

        // Note - must be async to avoid needing a result
        message::Message call;
        call.set_isasync(true);
        call.set_user("demo");
        call.set_function("echo");
        call.set_inputdata("abc");

        // Get global bus
        scheduler::GlobalMessageBus &globalBus = scheduler::getGlobalMessageBus();

        edge::FunctionEndpoint endpoint;
        endpoint.handleFunction(call);

        const message::Message actual = globalBus.nextMessage();

        REQUIRE(actual.user() == "demo");
        REQUIRE(actual.function() == "echo");
        REQUIRE(actual.inputdata() == "abc");
    }
}