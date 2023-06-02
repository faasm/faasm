#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/func.h>

namespace tests {

TEST_CASE_METHOD(FunctionExecTestFixture, "Test printf", "[wasm]")
{
    auto req = setUpContext("demo", "print");
    executeWithPool(req);
}

TEST_CASE_METHOD(FunctionExecTestFixture, "Test emscripten I/O", "[wasm]")
{
    auto req = setUpContext("demo", "emscripten_check");
    executeWithPool(req);
}
}
