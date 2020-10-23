#include <catch2/catch.hpp>

#include <faabric/util/environment.h>
#include <faabric/util/config.h>

#include <system/NetworkNamespace.h>

using namespace isolation;

namespace tests {

    TEST_CASE("Test basic network properties", "[faaslet]") {
        faabric::util::SystemConfig &conf = faabric::util::getSystemConfig();
        std::string original = conf.netNsMode;
        
        std::string envValue;
        NetworkIsolationMode expected;
        SECTION("Test network namespace on") {
            envValue = "on";
            expected = NetworkIsolationMode::ns_on;
        }

        SECTION("Test network namespace off") {
            envValue = "off";
            expected = NetworkIsolationMode::ns_off;
        }

        // Reset config
        faabric::util::setEnvVar("NETNS_MODE", envValue);
        conf.reset();

        // Create and check namespace
        NetworkNamespace ns("foo");
        REQUIRE(ns.getMode() == expected);
        REQUIRE(ns.getName() == "foo");

        // Reset conf
        faabric::util::setEnvVar("NETNS_MODE", original);
        conf.reset();
    }
}
