#include <catch/catch.hpp>

#include <faabric/util/random.h>

#include <set>

using namespace faabric::util;

namespace tests {
    TEST_CASE("Test random string generation", "[util]") {
        std::string actualA = randomString(100);
        REQUIRE(actualA.size() == 100);

        std::string actualB = randomString(100);
        REQUIRE(actualB.size() == 100);

        REQUIRE(actualA != actualB);
    }


    TEST_CASE("Test random value from set", "[random]") {
        std::unordered_set<std::string> s;

        // Should return empty string if nothing
        REQUIRE(faabric::utilrandomStringFromSet(s).empty());

        s.insert("foo");
        s.insert("bar");
        s.insert("baz");
        s.insert("qux");

        std::unordered_set<std::string> actual;
        for(int i = 0; i < 1000; i++) {
            actual.insert(faabric::utilrandomStringFromSet(s));
        }

        REQUIRE(actual.size() == 4);
    }
}
