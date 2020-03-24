#include <catch/catch.hpp>

#include <storage/IBMStorageWrapper.h>
#include <util/config.h>

using namespace storage;

namespace tests {
    TEST_CASE("Test authentication", "[ibm]") {
        util::SystemConfig &conf = util::getSystemConfig();
        IBMStorageWrapper wrapper(conf.ibmApiKey);

        const std::string &actual = wrapper.getAuthToken();
        REQUIRE(!actual.empty());
    }

    TEST_CASE("Test read/write keys in bucket", "[ibm]") {
        util::SystemConfig &conf = util::getSystemConfig();
        IBMStorageWrapper wrapper(conf.ibmApiKey);

        std::vector<uint8_t> byteDataA = {0, 1, 2, 3, 'c', '\\', '@', '$', '%'};
        std::vector<uint8_t> byteDataB = {11, 99, 123, '#', '\n', '\t'};

        SECTION("Test bytes read/ write") {
            wrapper.addKeyBytes(conf.bucketName, "alpha", byteDataA);
            wrapper.addKeyBytes(conf.bucketName, "beta", byteDataB);

            REQUIRE(wrapper.getKeyBytes(conf.bucketName, "alpha") == byteDataA);
            REQUIRE(wrapper.getKeyBytes(conf.bucketName, "beta") == byteDataB);
        }
    }
}