#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

#include <faabric/util/func.h>

namespace tests {
TEST_CASE_METHOD(FunctionExecTestFixture,
                 "Test zygote function works",
                 "[faaslet]")
{
    auto req = setUpContext("demo", "zygote_check");
    execFunction(req);
}

TEST_CASE_METHOD(FunctionExecTestFixture,
                 "Test repeat execution of zygote function",
                 "[faaslet]")
{
    auto req = setUpContext("demo", "zygote_check");
    checkMultipleExecutions(req->mutable_messages()->at(0), 4);
}
}
