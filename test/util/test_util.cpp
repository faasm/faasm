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
    
    TEST_CASE("Test converting integers to bytes", "[util]") {
        int ints[3] = {1, 2, 3};
        uint8_t bytes[12];

        intToBytes(ints, 3, bytes);

        REQUIRE(bytes[0] == 1);
        REQUIRE(bytes[1] == 0);
        REQUIRE(bytes[2] == 0);
        REQUIRE(bytes[3] == 0);

        REQUIRE(bytes[4] == 2);
        REQUIRE(bytes[5] == 0);
        REQUIRE(bytes[6] == 0);
        REQUIRE(bytes[7] == 0);

        REQUIRE(bytes[8] == 3);
        REQUIRE(bytes[9] == 0);
        REQUIRE(bytes[10] == 0);
        REQUIRE(bytes[11] == 0);
    }

}