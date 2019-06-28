#include <catch/catch.hpp>

#include <util/config.h>
#include <util/environment.h>

using namespace util;

namespace tests {

    TEST_CASE("Test default environment variables", "[util]") {
        std::string key = "JUNK_VAR";

        // Sanity check for null pointer when env var not set
        char const *val = getenv(key.c_str());
        REQUIRE(val == nullptr);

        REQUIRE(getEnvVar(key, "blah") == "blah");
    }

    TEST_CASE("Test empty environment variables", "[util]") {
        util::unsetEnvVar("MY_VAR");

        // Sanity check for empty string when env var set to empty
        char *currentValue = getenv("MY_VAR");
        REQUIRE(currentValue == nullptr);

        REQUIRE(getEnvVar("MY_VAR", "alpha") == "alpha");
    }
}