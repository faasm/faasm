#include <catch/catch.hpp>

#include <edge/edge.h>

using namespace Pistache;

namespace tests {
    TEST_CASE("Test invoking a function", "[edge]") {
        infra::Redis cli;
        cli.flushAll();

        // Make sure there's a worker queue in the unassigned queue
        const std::string workerQueue = "dummyQ";
        cli.addToUnassignedSet(workerQueue);

        // Note - must be async to avoid needing a result
        message::FunctionCall call;
        call.set_isasync(true);
        call.set_user("foo");
        call.set_function("bar");
        call.set_inputdata("abc");

        edge::FunctionEndpoint endpoint;
        endpoint.handleFunction(call);

        const message::FunctionCall actual = cli.nextFunctionCall(workerQueue);

        REQUIRE(actual.user() == "foo");
        REQUIRE(actual.function() == "bar");
        REQUIRE(actual.inputdata() == "abc");
    }
}