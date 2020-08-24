#include <catch/catch.hpp>
#include <faabric/util/network.h>

using namespace faabric::util;

namespace tests {
    TEST_CASE("Test getting primary IP", "[util]") {
        // Can only really check this is not empty
        std::string ipA = getPrimaryIPForThisHost("");
        REQUIRE(!ipA.empty());

        // Test IP on junk interface is empty
        std::string ipB = getPrimaryIPForThisHost("foobar");
        REQUIRE(ipB.empty());
    }
}
