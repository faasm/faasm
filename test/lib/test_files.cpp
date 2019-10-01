#include <catch/catch.hpp>
#include <util/files.h>
#include "utils.h"

#include "faasm/files.h"


namespace tests {
    TEST_CASE("Test reading file to string", "[files]") {
        // Fixed path
        std::string path("/etc/hosts");

        const char* actual = faasm::readFileToString(path.c_str());

        std::string expected = util::readFileToString(path);

        REQUIRE(actual == expected);
    }
}