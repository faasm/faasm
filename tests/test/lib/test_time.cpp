#include <catch2/catch.hpp>

#include "faasm/time.h"
#include <unistd.h>

using namespace faasm;

namespace tests {
TEST_CASE("Test epoch millis", "[time]")
{
    double actualA = getSecondsSinceEpoch();

    // Microseconds changes should be picked up
    unsigned int microseconds = 1000;
    usleep(microseconds);

    double actualB = getSecondsSinceEpoch();

    REQUIRE(actualB > actualA);
    REQUIRE(actualA > 1543912843);
}
}
