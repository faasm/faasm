#include "utils.h"

#include <catch2/catch.hpp>
#include <faabric/util/config.h>
#include <faabric/util/func.h>

using namespace wasm;

namespace tests {
TEST_CASE("Test executing hello function with SGX", "[sgx]")
{
    executeWithSGX("demo", "hello");
}

TEST_CASE("Test executing chaining by name with SGX", "[sgx]")
{
    executeWithSGX("demo", "chain_named_a");
}
}
