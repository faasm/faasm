#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

#include <faabric/util/config.h>
#include <faabric/util/func.h>

using namespace wasm;

namespace tests {
TEST_CASE_METHOD(FunctionExecTestFixture,
                 "Test executing hello function with SGX",
                 "[sgx]")
{
    executeWithSGX("demo", "hello");
}

TEST_CASE_METHOD(FunctionExecTestFixture,
                 "Test executing chaining by name with SGX",
                 "[sgx]")
{
    executeWithSGX("demo", "chain_named_a", 10000);
}
}
