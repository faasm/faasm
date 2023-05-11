#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

#include <faabric/util/func.h>

namespace tests {
TEST_CASE_METHOD(FunctionExecTestFixture, "Test memcpy", "[faaslet]")
{
    auto req = setUpContext("demo", "memcpy");
    executeWithPool(req);
}

TEST_CASE_METHOD(FunctionExecTestFixture, "Test memmove", "[faaslet]")
{
    auto req = setUpContext("demo", "memmove");
    executeWithPool(req);
}

TEST_CASE_METHOD(FunctionExecTestFixture, "Test calloc", "[faaslet]")
{
    auto req = setUpContext("demo", "calloc");
    executeWithPool(req);
}
}
