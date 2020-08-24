#include <catch/catch.hpp>
#include <wasm/WasmModule.h>
#include <faabric/util/bytes.h>
#include <faabric/util/func.h>
#include <faabric/util/config.h>


namespace tests {

    TEST_CASE("Test executing Typescript module with input and output", "[wasm]") {
        faabric::Message call = faabric::utilmessageFactory("ts", "echo");
        call.set_istypescript(true);

        // TODO - fix typescript
//        std::string inputData = "Typescript echo test";
//        call.set_inputdata(inputData);
//
//        wasm::WasmModule module;
//        module.bindToFunction(call);
//
//        // Execute the function
//        int result = module.execute(call);
//        REQUIRE(result == 0);
//
//        // Check output data
//        REQUIRE(inputData == call.outputdata());
    }
}
