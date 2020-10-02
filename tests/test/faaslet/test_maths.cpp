#include <catch2/catch.hpp>

#include "utils.h"

namespace tests {
TEST_CASE("Test BLAS support", "[faaslet]")
{
    cleanSystem();
    faabric::Message msg = faabric::util::messageFactory("demo", "blas");
    // TODO - fix BLAS test
    // execFunction(msg);
}
}
