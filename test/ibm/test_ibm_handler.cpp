#include <catch/catch.hpp>

#include "utils.h"
#include <ibm/IBMEndpoint.h>

#include <scheduler/GlobalMessageBus.h>

using namespace Pistache;

namespace tests {
    TEST_CASE("Test ibm initialisation", "[ibm]") {
        cleanSystem();

        // Set up config to something that we know will change
        util::SystemConfig &conf = util::getSystemConfig();
        std::string originalStorage = conf.functionStorage;
        std::string originalCGroup = conf.cgroupMode;
        std::string originalNetns = conf.netNsMode;

        conf.functionStorage = "foobar";
        conf.cgroupMode = "blah";
        conf.netNsMode = "blahblah";

        std::string requestStr;
        ibm::IBMEndpoint endpoint;
        endpoint.doHandleInit(requestStr);

        REQUIRE(conf.functionStorage == "ibm");
        REQUIRE(conf.cgroupMode == "off");
        REQUIRE(conf.netNsMode == "off");

        conf.functionStorage = originalStorage;
        conf.cgroupMode = originalCGroup;
        conf.netNsMode = originalNetns;
    }
}