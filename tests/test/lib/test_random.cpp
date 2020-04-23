#include <catch/catch.hpp>

#include "faasm/random.h"

using namespace faasm;

    namespace tests {
    TEST_CASE("Test random integer", "[random]") {
        // Slightly sketch to be testing a random number like this but probably a necessary evil.
        int topEnd = 1000;
        int a = randomInteger(10, topEnd);
        int b = randomInteger(30, topEnd);
        int c = randomInteger(100, topEnd);

        REQUIRE(a >= 10);
        REQUIRE(a <= topEnd);

        REQUIRE(b >= 30);
        REQUIRE(b <= topEnd);

        REQUIRE(c >= 100);
        REQUIRE(c <= topEnd);

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
