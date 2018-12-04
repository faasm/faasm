#include <catch/catch.hpp>

#include "faasm/time.h"
#include <unistd.h>

using namespace faasm;

namespace tests {
    TEST_CASE("Test epoch millis", "[time]") {
        long actualA = getMillisSinceEpoch();

        unsigned int microseconds = 2000;
        usleep(microseconds);

        long actualB = getMillisSinceEpoch();

        REQUIRE(actualB > actualA);
        REQUIRE(actualA > 1543912843186);
    }
}