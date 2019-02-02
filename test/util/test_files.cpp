#include <catch/catch.hpp>
#include <util/files.h>

#include <boost/filesystem.hpp>

using namespace util;

namespace tests {
    TEST_CASE("Test writing to a file", "[util]") {
        std::string dummyFile = "/tmp/faasmTest1.txt";

        // Write to the file
        std::vector<uint8_t> bytesIn = {0, 1, 2, 10, 20};
        util::writeBytesToFile(dummyFile, bytesIn);

        // Read in
        std::vector<uint8_t> actual = util::readFileToBytes(dummyFile);

        // Check they match
        REQUIRE(actual == bytesIn);
    }
}