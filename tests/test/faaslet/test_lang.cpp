#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

#include <faabric/util/func.h>

namespace tests {
TEST_CASE_METHOD(FunctionExecTestFixture,
                 "Test pointer to pointer",
                 "[faaslet]")
{
    auto req = setUpContext("demo", "ptr_ptr");
    executeWithPool(req);
}

TEST_CASE_METHOD(FunctionExecTestFixture,
                 "Test long double print doesn't fail",
                 "[faaslet]")
{
    auto req = setUpContext("demo", "long_double");
    executeWithPool(req);
}

TEST_CASE_METHOD(FunctionExecTestFixture, "Test sizes", "[faaslet]")
{
    auto req = setUpContext("demo", "sizes");
    executeWithPool(req);
}

TEST_CASE_METHOD(FunctionExecTestFixture, "Test stack/ heap", "[faaslet]")
{
    auto req = setUpContext("demo", "stackheap");
    executeWithPool(req);
}

TEST_CASE_METHOD(FunctionExecTestFixture, "Test backtrace", "[faaslet]")
{
    auto req = setUpContext("demo", "backtrace");
    executeWithPool(req);
}

TEST_CASE_METHOD(FunctionExecTestFixture, "Test varargs", "[faaslet]")
{
    auto req = setUpContext("demo", "va_arg");
    executeWithPool(req);
}
}
