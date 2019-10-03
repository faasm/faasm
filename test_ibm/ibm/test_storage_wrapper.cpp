#include <catch/catch.hpp>

#include <ibm/IBMStorageWrapper.h>
#include <util/config.h>

using namespace ibm;

namespace tests {
    TEST_CASE("Test authentication", "[ibm]") {
        util::SystemConfig &conf = util::getSystemConfig();
        IBMStorageWrapper wrapper(conf.ibmApiKey, conf.ibmStorageId);

        const std::string &actual = wrapper.getAuthToken();
        REQUIRE(!actual.empty());
    }
}