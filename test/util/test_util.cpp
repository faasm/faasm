#include <catch/catch.hpp>
#include <util/util.h>

using namespace util;

namespace tests {

    TEST_CASE("Test default environment variables", "[util]") {
        std::string key = "JUNK_VAR";

        // Sanity check for null pointer when env var not set
        char const *val = getenv(key.c_str());
        REQUIRE(val == nullptr);

        REQUIRE(getEnvVar(key, "blah") == "blah");
    }

    TEST_CASE("Test valid environment variables", "[util]") {
        std::string actual = getEnvVar("LANGUAGE", "blah");

        bool check = (actual == "en_GB:en") || (actual == "en_GB.utf8");
        REQUIRE(check);
    }

    TEST_CASE("Test empty environment variables", "[util]") {
        char dummyEnv[] = "MY_VAR=";
        putenv(dummyEnv);

        // Sanity check for empty string when env var set to empty
        char key[] = "MY_VAR";
        std::string realValue(getenv(key));
        REQUIRE(realValue.empty());

        REQUIRE(getEnvVar("MY_VAR", "alpha") == "alpha");
    }

}