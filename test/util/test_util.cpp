#include <catch/catch.hpp>
#include <util/util.h>

namespace tests {

    TEST_CASE("Test environment variables", "[util]") {
        REQUIRE(util::getEnvVar("JUNK_VAR", "blah") == "blah");
        REQUIRE(util::getEnvVar("LANGUAGE", "blah") == "en_GB:en");
    }

}