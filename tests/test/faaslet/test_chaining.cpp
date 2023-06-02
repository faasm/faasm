#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

#include <faabric/util/environment.h>

namespace tests {
TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test function chaining by pointer",
                 "[faaslet]")
{
    SECTION("WAVM") { conf.wasmVm = "wavm"; }

    SECTION("WAMR") { conf.wasmVm = "wamr"; }

    auto req = faabric::util::batchExecFactory("demo", "chain", 1);
    executeWithPool(req, 5000);
}

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test function chaining by name",
                 "[faaslet]")
{
    SECTION("WAVM") { conf.wasmVm = "wavm"; }

    SECTION("WAMR") { conf.wasmVm = "wamr"; }

    auto req = faabric::util::batchExecFactory("demo", "chain_named_a", 1);
    executeWithPool(req, 5000);
}
}
