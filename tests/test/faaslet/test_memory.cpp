#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

#include <faabric/util/func.h>

namespace tests {
TEST_CASE_METHOD(FunctionExecTestFixture, "Test memcpy", "[faaslet]")
{
    auto req = setUpContext("demo", "memcpy");
    execFunction(req);
}

TEST_CASE_METHOD(FunctionExecTestFixture, "Test memmove", "[faaslet]")
{
    auto req = setUpContext("demo", "memmove");
    execFunction(req);
}

// 23/06/2022 - This test is leaking so much memory that it is making the
// sanitised builds fail.
TEST_CASE_METHOD(FunctionExecTestFixture, "Test calloc", "[.]")
{
    auto req = setUpContext("demo", "calloc");
    execFunction(req);
}
}
