#include <catch2/catch.hpp>

#include <faabric/util/config.h>
#include <faabric/util/environment.h>

#include <conf/FaasmConfig.h>
#include <system/NetworkNamespace.h>

using namespace isolation;

namespace tests {

TEST_CASE("Test basic network properties", "[faaslet][network]")
{
    conf::FaasmConfig& conf = conf::getFaasmConfig();
    std::string original = conf.netNsMode;

    std::string envValue;
    NetworkIsolationMode expected;
    SECTION("Test network namespace on")
    {
        envValue = "on";
        expected = NetworkIsolationMode::ns_on;
    }

    SECTION("Test network namespace off")
    {
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

TEST_CASE("Test running out of namespaces", "[faaslet][network]")
{
    conf::FaasmConfig& conf = conf::getFaasmConfig();
    int originalMaxNetNs = conf.maxNetNs;

    faabric::util::setEnvVar("MAX_NET_NAMESPACES", "0");
    conf.reset();

    assert(conf.maxNetNs == 0);

    // Try claiming namespaces and fail
    REQUIRE_THROWS(claimNetworkNamespace());

    // Reset conf
    faabric::util::setEnvVar("MAX_NET_NAMESPACES",
                             std::to_string(originalMaxNetNs));
    conf.reset();
}
}
