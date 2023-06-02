#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

#include <faabric/proto/faabric.pb.h>
#include <faabric/util/func.h>
#include <faabric/util/macros.h>
#include <faaslet/Faaslet.h>
#include <wavm/WAVMWasmModule.h>

namespace tests {

TEST_CASE_METHOD(FunctionExecTestFixture,
                 "Test creating cached WAVM modules",
                 "[wasm]")
{
    auto req = setUpContext("demo", "chain", 2);
    faabric::Message& msgA = req->mutable_messages()->at(0);
    faabric::Message& msgB = req->mutable_messages()->at(1);

    // Want to check things with chained calls, so need to fake up input to a
    // chained func
    // NOTE: here we are hard-coding the function's position in the function
    // table
    int funcPtr = 2;
    msgA.set_funcptr(funcPtr);

    int input[3] = { 1, 2, 3 };
    msgA.set_inputdata(BYTES(input), 3 * sizeof(int));

    // Note - we need to explicitly unlock these locks otherwise we get a
    // deadlock
    auto [moduleA, lockA] = moduleCache.getCachedModule(msgA);
    lockA.unlock();
    auto [moduleB, lockB] = moduleCache.getCachedModule(msgB);
    lockB.unlock();

    // Check modules are the same
    REQUIRE(std::addressof(moduleA) == std::addressof(moduleB));
    REQUIRE(moduleA.isBound());

    // Execute the function normally and make sure cached module is not used
    faaslet::Faaslet faaslet(msgA);
    int returnValue = faaslet.executeTask(0, 0, req);
    REQUIRE(returnValue == 0);

    REQUIRE(std::addressof(moduleA) != std::addressof(*faaslet.module));

    faaslet.shutdown();
}
}
