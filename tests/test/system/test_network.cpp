#include <catch2/catch.hpp>

#include "faasm_fixtures.h"

#include <faabric/util/config.h>
#include <faabric/util/environment.h>

#include <conf/FaasmConfig.h>
#include <system/NetworkNamespace.h>

using namespace isolation;

namespace tests {
TEST_CASE_METHOD(FaasmConfTestFixture,
                 "Test running out of namespaces",
                 "[faaslet][network]")
{
    conf.netNsMode = "on";

    // Drain the existing namespaces
    std::vector<std::shared_ptr<NetworkNamespace>> namespaces(conf.maxNetNs);
    for (int i = 0; i < conf.maxNetNs; i++) {
        namespaces.at(i) = claimNetworkNamespace();
    }

    // Try claiming another namespaces and fail
    REQUIRE_THROWS(claimNetworkNamespace());

    // Return all the claimed namespaces
    for (auto& ns : namespaces) {
        returnNetworkNamespace(ns);
    }
}
}
