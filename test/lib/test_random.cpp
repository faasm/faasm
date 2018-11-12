#include <catch/catch.hpp>

#include "faasm/random.h"

using namespace faasm;

namespace tests {
    TEST_CASE("Test random integer", "[random]") {
        // Slightly sketch to be testing a random number like this but probably a necessary evil.
        int a = randomInteger(10, 50);
        int b = randomInteger(10, 50);
        int c = randomInteger(10, 50);

        REQUIRE(a >= 10);
        REQUIRE(a < 50);

        REQUIRE(b >= 10);
        REQUIRE(b < 50);

        REQUIRE(c >= 10);
        REQUIRE(c < 50);

        bool allUnequal = (a != b) & (b != c);
        REQUIRE(allUnequal);
    }

    TEST_CASE("Test array shuffling", "[random]") {
        std::vector<int> arrayIn = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        std::vector<int> arrayInCopy = arrayIn;

        shuffleArray(arrayIn.data(), arrayIn.size());

        REQUIRE(arrayIn != arrayInCopy);
    }

    TEST_CASE("Test random int range", "[random]") {
        int *actual = randomIntRange(20);

        // Check all integers are present
        std::vector<int> actualVec(actual, actual + 20);
        for (int i = 0; i < 20; i++) {
            bool isPresent = std::find(actualVec.begin(), actualVec.end(), i) != actualVec.end();
            REQUIRE(isPresent);
        }
    }
}
