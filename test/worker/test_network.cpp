#include <catch/catch.hpp>

#include <util/environment.h>
#include <util/config.h>

#include <worker/NetworkNamespace.h>

using namespace worker;

namespace tests {

    TEST_CASE("Test basic network properties", "[worker]") {
        util::SystemConfig &conf = util::getSystemConfig();

        util::setEnvVar("NETNS_MODE", "on");
        NetworkNamespace nsA("foo");

        conf.reset();
        util::setEnvVar("NETNS_MODE", "off");
        NetworkNamespace nsB("bar");

        REQUIRE(nsA.getMode() == NetworkIsolationMode::ns_on);
        REQUIRE(nsA.getName() == "foo");

        REQUIRE(nsB.getMode() == NetworkIsolationMode::ns_off);
        REQUIRE(nsB.getName() == "bar");

        util::unsetEnvVar("NETNS_MODE");
    }
}
