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

    TEST_CASE("Nothing yet", "[edge]") {
        edge::FunctionEndpoint endpoint = prepareEndpoint();

        REQUIRE(2 + 2 == 4);
    }
}