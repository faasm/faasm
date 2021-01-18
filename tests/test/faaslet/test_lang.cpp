#include <catch2/catch.hpp>

#include "utils.h"

#include <faabric/util/func.h>

namespace tests {
TEST_CASE("Test pointer to pointer", "[faaslet]")
{
    cleanSystem();
    faabric::Message msg = faabric::util::messageFactory("demo", "ptr_ptr");
    execFunction(msg);
}

TEST_CASE("Test long double print doesn't fail", "[faaslet]")
{
    cleanSystem();
    faabric::Message msg = faabric::util::messageFactory("demo", "long_double");
    execFunction(msg);
}

TEST_CASE("Test sizes", "[faaslet]")
{
    cleanSystem();
    faabric::Message msg = faabric::util::messageFactory("demo", "sizes");
    execFunction(msg);
}

TEST_CASE("Test stack/ heap", "[faaslet]")
{
    cleanSystem();
    faabric::Message msg = faabric::util::messageFactory("demo", "stackheap");
    execFunction(msg);
}

TEST_CASE("Test backtrace", "[faaslet]")
{
    cleanSystem();
    faabric::Message msg = faabric::util::messageFactory("demo", "backtrace");
    execFunction(msg);
}
}
