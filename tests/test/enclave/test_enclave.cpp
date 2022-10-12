#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

#include <faabric/util/config.h>
#include <faabric/util/func.h>

using namespace wasm;

namespace tests {
TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test executing hello function with SGX",
                 "[sgx]")
{
    auto req = setUpContext("demo", "hello");
    faabric::Message& msg = req->mutable_messages()->at(0);

    execSgxFunction(msg);
}

// TODO: this is not chaining by name?
TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test executing chaining by name with SGX",
                 "[sgx]")
{
    auto req = setUpContext("demo", "hello");
    faabric::Message& msg = req->mutable_messages()->at(0);

    execFuncWithSgxPool(msg, 10000);
}
}
