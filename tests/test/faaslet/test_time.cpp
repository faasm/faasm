#include <catch2/catch.hpp>

#include "utils.h"

namespace tests {
TEST_CASE("Test time progresses", "[faaslet]")
{
    cleanSystem();
    faabric::Message msg = faabric::util::messageFactory("demo", "gettime");
    execFunction(msg);
}
}
