#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

namespace tests {

TEST_CASE_METHOD(FunctionExecTestFixture, "Test time progresses", "[faaslet]")
{
    auto req = setUpContext("demo", "gettime");
    executeWithPool(req);
}
}
