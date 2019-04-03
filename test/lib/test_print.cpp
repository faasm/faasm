#include <catch/catch.hpp>

#include "faasm/print.h"

#include <string.h>

using namespace faasm;

namespace tests {
    void _checkFloatPrint(float in, const char *expected) {
        std::string expectedStr(expected);

        REQUIRE(expectedStr == floatToStr(in));
    }

    TEST_CASE("Test printing floats", "[print]") {
        // Simple
        _checkFloatPrint(2.56, "2.559");

        // Long decimals
        _checkFloatPrint(11.23456, "11.234");

        // Long integer part
        _checkFloatPrint(8888.23, "8888.230");

        // Negative
        _checkFloatPrint(-23.454, "-23.454");
    }

}