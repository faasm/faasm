#include <catch/catch.hpp>
#include <util/state.h>

using namespace util;

namespace tests {

    TEST_CASE("Test creating key for user", "[util]") {
        REQUIRE(util::keyForUser("foo", "bar") == "foo_bar");
    }
}