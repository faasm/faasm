#include <catch2/catch.hpp>

#include "utils.h"

namespace tests {
TEST_CASE("Test BLAS support", "[faaslet][!mayfail]")
{
    cleanSystem();
    faabric::Message msg = faabric::util::messageFactory("demo", "blas");

    execFunction(msg);
}
}
