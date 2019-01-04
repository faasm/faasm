#include <catch/catch.hpp>
#include <util/util.h>

using namespace util;

namespace tests {
    TEST_CASE("Test string splitting", "[util]") {
        std::string strA = "one string";
        std::string strB = "another string with other stuff";

        std::vector<std::string> expectedA = {"one", "string"};
        std::vector<std::string> expectedB = {"another", "string", "with", "other", "stuff"};

        REQUIRE(tokeniseString(strA, ' ') == expectedA);
        REQUIRE(tokeniseString(strB, ' ') == expectedB);
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
}