#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

#include <faabric/util/environment.h>

using namespace faaslet;

namespace tests {
TEST_CASE_METHOD(FunctionExecTestFixture, "Test function chaining", "[faaslet]")
{
    faabric::Message call = faabric::util::messageFactory("demo", "chain");
    execFuncWithPool(call, true, 5000);
}

TEST_CASE_METHOD(FunctionExecTestFixture,
                 "Test named function chaining",
                 "[faaslet]")
{
    faabric::Message call =
      faabric::util::messageFactory("demo", "chain_named_a");
    execFuncWithPool(call, true, 5000);
}

}
