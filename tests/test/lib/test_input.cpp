#include <catch/catch.hpp>

#include "faasm/input.h"

namespace tests {
    TEST_CASE("Test parse string to ints", "[input]") {
        const char *input = "0 123 99987 5";
        int* actual = faasm::parseStringToIntArray(input, 4);

        REQUIRE(actual[0] == 0);
        REQUIRE(actual[1] == 123);
        REQUIRE(actual[2] == 99987);
        REQUIRE(actual[3] == 5);

        delete[] actual;
    }
}