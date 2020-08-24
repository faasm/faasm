#include <catch/catch.hpp>
#include <wamr/WAMRWasmModule.h>
#include <util/func.h>
#include <util/config.h>

using namespace wasm;

namespace tests {
    TEST_CASE("Test executing echo function with WAMR", "[wasm]") {
        faabric::Message call = util::messageFactory("demo", "echo");
        std::string inputData = "hello there";
        call.set_inputdata(inputData);

        wasm::WAMRWasmModule module;
        module.bindToFunction(call);
        
        bool success = module.execute(call);
        REQUIRE(success);

        std::string outputData = call.outputdata();
        REQUIRE(outputData == inputData);
    }
}
