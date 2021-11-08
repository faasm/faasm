#include "utils.h"

#include <catch2/catch.hpp>

#include <conf/FaasmConfig.h>
#include <faabric/util/config.h>
#include <faabric/util/func.h>

using namespace wasm;

namespace tests {
TEST_CASE("Test executing hello function with SGX", "[sgx]")
{
    executeWithSGX("demo", "hello", 10000);
}

TEST_CASE("Test executing, flushing, and executing again with SGX", "[sgx3]")
{
    executeWithSGX("demo", "hello", 10000);

    // Set the WASM VM before flushing
    conf::FaasmConfig& conf = conf::getFaasmConfig();
    const std::string originalVm = conf.wasmVm;
    conf.wasmVm = "wamr";
    faabric::scheduler::getScheduler().flushLocally();
    conf.wasmVm = originalVm;

    executeWithSGX("demo", "hello", 10000);
}

TEST_CASE("Test executing chaining by name with SGX", "[sgx]")
{
    executeWithSGX("demo", "chain_named_a", 10000);
}
}
