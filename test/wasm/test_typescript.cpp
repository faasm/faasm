#include <catch/catch.hpp>
#include <wasm/WasmModule.h>
#include <util/bytes.h>
#include <util/func.h>
#include <util/config.h>


namespace tests {

    TEST_CASE("Test executing Typescript module with input and output", "[wasm]") {
        message::Message call = util::messageFactory("ts", "echo");
        call.set_istypescript(true);

        std::string inputData = "Typescript echo test";
        call.set_inputdata(inputData);

        wasm::WasmModule module;
        module.bindToFunction(call);

        // Execute the function
        int result = module.execute(call);
        REQUIRE(result == 0);

        // Check output data
        REQUIRE(inputData == call.outputdata());
    }
}
