#include "utils.h"

#include <catch2/catch.hpp>
#include <faabric/util/config.h>
#include <faabric/util/func.h>
#include <wamr/WAMRWasmModule.h>

using namespace wasm;

namespace tests {

TEST_CASE("Test executing echo function with WAMR", "[wamr]")
{
    faabric::Message call = faabric::util::messageFactory("demo", "echo");
    std::string inputData = "hello there";
    call.set_inputdata(inputData);

    wasm::WAMRWasmModule module;
    module.bindToFunction(call);

    bool success = module.execute(call);
    REQUIRE(success);

    std::string outputData = call.outputdata();
    REQUIRE(outputData == inputData);
}

TEST_CASE("Test WAMR sbrk", "[wamr]")
{
    faabric::Message call = faabric::util::messageFactory("demo", "echo");
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

TEST_CASE("Test executing chain function with WAMR", "[wamr]")
{
    executeWithWamrPool("demo", "chain");
}
}
