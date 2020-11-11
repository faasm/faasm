#include <catch2/catch.hpp>

#include "utils.h"

#include <faabric/util/func.h>

namespace tests {
TEST_CASE("Test memcpy", "[faaslet]")
{
    cleanSystem();
    faabric::Message msg = faabric::util::messageFactory("demo", "memcpy");
    execFunction(msg);
}

TEST_CASE("Test memmove", "[faaslet]")
{
    cleanSystem();
    faabric::Message msg = faabric::util::messageFactory("demo", "memcpy");
    execFunction(msg);
}

TEST_CASE("Test calloc", "[faaslet]")
{
    cleanSystem();
    faabric::Message msg = faabric::util::messageFactory("demo", "memcpy");
    execFunction(msg);
}
}
