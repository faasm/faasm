#include <catch/catch.hpp>
#include <faabric/util/hash.h>

using namespace faabric::util;

namespace tests {
    TEST_CASE("Test hashing a string", "[util]") {
        std::string strA = "one string";
        std::string strB = "another string";

        // Check hashes are different and repeatable
        std::string hashA1 = stringToSHA1(strA);
        std::string hashA2 = stringToSHA1(strA);

        std::string hashB1 = stringToSHA1(strB);
        std::string hashB2 = stringToSHA1(strB);

        REQUIRE(hashA1 == hashA2);
        REQUIRE(hashA1 != hashB1);
        REQUIRE(hashB1 == hashB2);
    }
}