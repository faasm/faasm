#include <catch2/catch.hpp>

#include "faasm_fixtures.h"

#include <faaslet/Faaslet.h>

namespace tests {
TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test exceptions are propagated from handler to runtime",
                 "[faaslet]")
{
    SECTION("WAVM") { conf.wasmVm = "wavm"; }

    SECTION("WAVM") { conf.wasmVm = "wamr"; }

    // TODO: make this default executeFunction in utils
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("demo", "exception", 1);
    faabric::Message& msg = req->mutable_messages()->at(0);
    faabric::scheduler::ExecutorContext::set(nullptr, req, 0);
    faaslet::Faaslet f(msg);

    REQUIRE_THROWS_AS(f.executeTask(0, 0, req),
                      faabric::util::FunctionMigratedException);
}
}
