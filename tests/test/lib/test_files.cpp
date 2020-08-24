#include <catch/catch.hpp>
#include <faabric/util/files.h>
#include "utils.h"

#include "faasm/files.h"


namespace tests {
    TEST_CASE("Test reading file to string", "[files]") {
        // Fixed path
        std::string path("/etc/hosts");

        // Read with utils function
        std::string expected = faabric::utilreadFileToString(path);

        // Check length is being reported properly
        long actualLength = faasm::getFileLength(path.c_str());
        REQUIRE(actualLength == expected.size());

        // Read with Faasm function
        const char* actual = faasm::readFileToString(path.c_str());

        // Check contents
        REQUIRE(actual == expected);
    }
}