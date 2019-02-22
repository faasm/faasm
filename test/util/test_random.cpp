#include <catch/catch.hpp>

#include "util/random.h"

using namespace util;

namespace tests {
    TEST_CASE("Test random string generation", "[util]") {
        std::string actualA = randomString(100);
        REQUIRE(actualA.size() == 100);

        std::string actualB = randomString(100);
        REQUIRE(actualB.size() == 100);

        REQUIRE(actualA != actualB);
    }
}
