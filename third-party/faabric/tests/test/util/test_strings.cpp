#include <catch/catch.hpp>
#include <util/string_tools.h>
#include <util/bytes.h>

using namespace util;

namespace tests {
    TEST_CASE("Test bytes to string round trip", "[util]") {
        std::string inputStr = "abcdefghijkl12345";

        const std::vector<uint8_t> bytes = util::stringToBytes(inputStr);
        REQUIRE(bytes.size() == inputStr.size());

        std::string actual = util::bytesToString(bytes);
        REQUIRE(actual.size() == inputStr.size());

        REQUIRE(actual == inputStr);
    }
    
    TEST_CASE("Test string splitting", "[util]") {
        std::string strA = "  one string";
        std::string strB = "another string with other stuff  ";

        std::vector<std::string> expectedA = {"one", "string"};
        std::vector<std::string> expectedB = {"another", "string", "with", "other", "stuff"};

        REQUIRE(splitString(strA, ' ') == expectedA);
        REQUIRE(splitString(strB, ' ') == expectedB);
    }

    TEST_CASE("Test is all whitespace", "[util]") {
        REQUIRE(isAllWhitespace("    "));
        REQUIRE(!isAllWhitespace("  s  "));
    }

    TEST_CASE("Test startswith", "[util]") {
        REQUIRE(startsWith("foobar", "foo"));
        REQUIRE(!startsWith("foobar", "goo"));
        REQUIRE(!startsWith("foobar", ""));
    }

    TEST_CASE("Test endswith", "[util]") {
        REQUIRE(endsWith("foobar", "bar"));
        REQUIRE(!endsWith("foobar", "foo"));
        REQUIRE(!endsWith("foobar", "ob"));
        REQUIRE(!endsWith("foobar", ""));
        REQUIRE(!endsWith("", "foobar"));
    }

    TEST_CASE("Test remove substr", "[util]") {
        REQUIRE(removeSubstr("blah foobar", "blah") == " foobar");
        REQUIRE(removeSubstr("blahblah", "") == "blahblah");
        REQUIRE(removeSubstr("", "foobar") == "");
        REQUIRE(removeSubstr("foo bar baz", "bar") == "foo  baz");
    }

    TEST_CASE("Test string is int", "[util]") {
        REQUIRE(stringIsInt("12345"));
        REQUIRE(stringIsInt("0"));

        REQUIRE(!stringIsInt(" 12345"));
        REQUIRE(!stringIsInt("123 "));
        REQUIRE(!stringIsInt("abcd"));
        REQUIRE(!stringIsInt("12a33"));
    }
}