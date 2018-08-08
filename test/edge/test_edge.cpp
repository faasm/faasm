#include <catch/catch.hpp>
#include <pistache/http.h>

#include <edge/edge.h>
#include <proto/faasm.pb.h>

namespace tests {

    edge::FunctionEndpoint prepareEndpoint() {
        Pistache::Port port(8001);
        Pistache::Address addr(Ipv4::any(), port);

        edge::FunctionEndpoint endpoint(addr);
        endpoint.init(1);

        return endpoint;
    }

    TEST_CASE("Test edge calling a function", "[edge]") {
        edge::FunctionEndpoint endpoint = prepareEndpoint();

        // Prepare the call
        message::FunctionCall call;
        call.set_user("jimmy");
        call.set_function("myfun");
        call.set_inputdata("foobar data");

        // Check resultkey not set initially
        REQUIRE(!call.has_resultkey());

        // Make the call
        endpoint.callFunction(call);

        // Get the call from the queue
        infra::RedisClient cli;
        cli.connect();
        message::FunctionCall actual = cli.nextFunctionCall();

        REQUIRE("jimmy" == actual.user());
        REQUIRE("myfun" == actual.function());
        REQUIRE("foobar data" == actual.inputdata());

        // Check result key has now been set
        REQUIRE(actual.has_resultkey());
    }
}