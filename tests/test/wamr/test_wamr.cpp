#include "utils.h"

#include <catch2/catch.hpp>
#include <faabric/util/config.h>
#include <faabric/util/func.h>
#include <wamr/WAMRWasmModule.h>

using namespace wasm;

namespace tests {

void executeWithWamrPool(const std::string& user, const std::string& func)
{
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    const std::string originalVm = conf.wasmVm;
    conf.wasmVm = "wamr";

    faabric::Message call = faabric::util::messageFactory("demo", "chain");

    // Don't clean so that the WAMR configuration persists
    execFuncWithPool(call, false, 1, false, 5, false);

    conf.wasmVm = originalVm;
}

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

TEST_CASE("Test executing chain function with WAMR", "[wamr]")
{
    executeWithWamrPool("demo", "chain");
}
}
