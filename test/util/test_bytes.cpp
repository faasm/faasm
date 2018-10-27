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

        std::string expectedA("en_GB:en");
        std::string expectedB("en_GB:utf8");

        bool isEqual = (actual == expectedA) || (actual == expectedB);
        REQUIRE(isEqual);
    }

    TEST_CASE("Test empty environment variables", "[util]") {
        util::unsetEnvVar("MY_VAR");

        // Sanity check for empty string when env var set to empty
        char *currentValue = getenv("MY_VAR");
        REQUIRE(currentValue == nullptr);

        REQUIRE(getEnvVar("MY_VAR", "alpha") == "alpha");
    }
    
    TEST_CASE("Test converting integers to bytes", "[util]") {
        std::string input = "abcde";

        std::vector<uint8_t> actual = stringToBytes(input);

        // Check cast back to char
        REQUIRE('a' == (char)actual[0]);
        REQUIRE('b' == (char)actual[1]);
        REQUIRE('c' == (char)actual[2]);
        REQUIRE('d' == (char)actual[3]);
    }

    TEST_CASE("Test removing trailing zeros", "[util]") {
        std::vector<uint8_t> input = {0, 2, 10, 0, 32, 0, 0, 0, 0};

        util::trimTrailingZeros(input);

        REQUIRE(input.size() == 5);
        std::vector<uint8_t> expected = {0, 2, 10, 0, 32};
        REQUIRE(input == expected);
    }

    TEST_CASE("Test removing trailing zeros on all zeros", "[util]") {
        std::vector<uint8_t> input = {0, 0, 0, 0, 0};

        util::trimTrailingZeros(input);

        REQUIRE(input.empty());
    }

    TEST_CASE("Test safe copy to small buffer", "[util]") {
        std::vector<uint8_t> buffer = {0, 1, 2, 3, 4, 5, 6};

        uint8_t smallBuff[3];
        safeCopyToBuffer(buffer, smallBuff, 3);

        REQUIRE(smallBuff[0] == 0);
        REQUIRE(smallBuff[1] == 1);
        REQUIRE(smallBuff[2] == 2);
    }

    TEST_CASE("Test safe copy to big buffer", "[util]") {
        std::vector<uint8_t> buffer = {0, 1, 2, 3, 4, 5, 6};

        uint8_t bigBuff[20];
        safeCopyToBuffer(buffer, bigBuff, 20);

        std::vector<uint8_t> actual(bigBuff, bigBuff + 7);
        REQUIRE(actual == buffer);
    }

}