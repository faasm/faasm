#include <catch2/catch.hpp>

#include "faasm_fixtures.h"
#include "utils.h"

#include <faabric/util/config.h>
#include <faabric/util/func.h>

#include <conf/FaasmConfig.h>
#include <wamr/WAMRWasmModule.h>

using namespace wasm;

namespace tests {

TEST_CASE_METHOD(MultiRuntimeFunctionExecTestFixture,
                 "Test executing simple function with WAMR",
                 "[wamr]")
{
    int nExecs = 0;
    std::string function;

    SECTION("echo function")
    {
        function = "echo";

        SECTION("Once") { nExecs = 1; }

        SECTION("Multiple") { nExecs = 5; }
    }

    // We must also check a function that changes the memory size to check that
    // it doesn't mess up
    SECTION("brk function")
    {
        function = "brk";

        SECTION("Once") { nExecs = 1; }

        SECTION("Multiple") { nExecs = 5; }
    }

    // Set to run WAMR
    conf.wasmVm = "wamr";

    // Create a Faaslet
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("demo", function, 1);
    faabric::Message& msg = req->mutable_messages()->at(0);
    faabric::scheduler::ExecutorContext::set(nullptr, req, 0);
    faaslet::Faaslet f(msg);

    // Execute the function using another message
    for (int i = 0; i < nExecs; i++) {
        std::shared_ptr<faabric::BatchExecuteRequest> req =
          faabric::util::batchExecFactory("demo", function, 1);
        faabric::Message& msg = req->mutable_messages()->at(0);
        faabric::scheduler::ExecutorContext::set(nullptr, req, 0);

        std::string inputData = fmt::format("hello there {}", i);
        msg.set_inputdata(inputData);

        int returnValue = f.executeTask(0, 0, req);
        REQUIRE(returnValue == 0);

        REQUIRE(msg.returnvalue() == 0);

        if (function == "echo") {
            std::string outputData = msg.outputdata();
            REQUIRE(outputData == inputData);
        }
    }

    f.shutdown();
}

TEST_CASE_METHOD(FunctionExecTestFixture, "Test WAMR sbrk", "[wamr]")
{
    auto req = setUpContext("demo", "echo");
    faabric::Message& call = req->mutable_messages()->at(0);
    std::string inputData = "hello there";
    call.set_inputdata(inputData);

    wasm::WAMRWasmModule module;
    module.bindToFunction(call);

    size_t initialSize = module.getMemorySizeBytes();
    REQUIRE(module.getCurrentBrk() == initialSize);

    uint32_t growA = 5 * WASM_BYTES_PER_PAGE;
    uint32_t growB = 20 * WASM_BYTES_PER_PAGE;

    module.growMemory(growA);
    size_t sizeA = module.getMemorySizeBytes();
    REQUIRE(sizeA > initialSize);
    REQUIRE(sizeA == initialSize + growA);
    REQUIRE(module.getCurrentBrk() == sizeA);

    module.growMemory(growB);
    size_t sizeB = module.getMemorySizeBytes();
    REQUIRE(sizeB > initialSize + growA);
    REQUIRE(sizeB == initialSize + growA + growB);
    REQUIRE(module.getCurrentBrk() == sizeB);
}

TEST_CASE_METHOD(FunctionExecTestFixture,
                 "Test executing chain function with WAMR",
                 "[wamr]")
{
    executeWithWamrPool("demo", "chain", 10000);
}
}
