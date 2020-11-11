#include <catch2/catch.hpp>

#include "utils.h"

#include <faabric/util/func.h>

namespace tests {
TEST_CASE("Test zygote function works", "[faaslet]")
{
    cleanSystem();
    faabric::Message msg =
      faabric::util::messageFactory("demo", "zygote_check");
    execFunction(msg);
}

TEST_CASE("Test repeat execution of zygote function", "[faaslet]")
{
    cleanSystem();
    faabric::Message msg =
      faabric::util::messageFactory("demo", "zygote_check");
    checkMultipleExecutions(msg, 4);
}
}
