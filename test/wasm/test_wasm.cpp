#include <catch/catch.hpp>
#include <wasm/wasm.h>
#include <util/util.h>

namespace tests {

    TEST_CASE("Test executing WASM module with no input", "[wasm]") {
        message::FunctionCall call;
        call.set_user("demo");
        call.set_function("dummy");

        wasm::WasmModule module;

        // Execute the function
        int result = module.execute(call);
        REQUIRE(result == 0);

        std::string outputData = call.outputdata();
        const std::vector<uint8_t> outputBytes = util::stringToBytes(outputData);
        
        // Check output data
        REQUIRE(outputBytes[0] == 0);
        REQUIRE(outputBytes[1] == 1);
        REQUIRE(outputBytes[2] == 2);
        REQUIRE(outputBytes[3] == 3);

        module.clean();
    }

    TEST_CASE("Test executing WASM module with input and output", "[wasm]") {
        message::FunctionCall call;
        call.set_user("demo");
        call.set_function("x2");

        wasm::WasmModule module;

        // Build input as byte stream
        U8 inputValues[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        call.set_inputdata(inputValues, 10);

        // Make the call
        int result = module.execute(call);
        REQUIRE(result == 0);

        std::string outputData = call.outputdata();
        const std::vector<uint8_t> outputBytes = util::stringToBytes(outputData);

        // Check the results
        std::vector<uint8_t > expected = {0, 2, 4, 6, 8, 10, 12, 14, 16, 18};
        REQUIRE(outputBytes == expected);

        module.clean();
    }
}
