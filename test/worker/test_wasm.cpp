#include <catch/catch.hpp>
#include <worker/worker.h>
#include <util/util.h>

namespace tests {

    TEST_CASE("Test executing WASM module with no input", "[worker]") {
        message::FunctionCall call;
        call.set_user("simon");
        call.set_function("dummy");

        worker::WasmModule module;

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

    TEST_CASE("Test executing WASM module with input and output", "[worker]") {
        message::FunctionCall call;
        call.set_user("simon");
        call.set_function("x2");

        worker::WasmModule module;

        // Build input as byte stream
        U8 inputValues[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        call.set_inputdata(inputValues, 10);

        // Make the call
        int result = module.execute(call);
        REQUIRE(result == 0);

        std::string outputData = call.outputdata();
        const std::vector<uint8_t> outputBytes = util::stringToBytes(outputData);

        // Check the results
        REQUIRE(outputData[0] == 0);
        REQUIRE(outputData[1] == 2);
        REQUIRE(outputData[2] == 4);
        REQUIRE(outputData[3] == 6);
        REQUIRE(outputData[4] == 8);
        REQUIRE(outputData[5] == 10);
        REQUIRE(outputData[6] == 12);
        REQUIRE(outputData[7] == 14);
        REQUIRE(outputData[8] == 16);
        REQUIRE(outputData[9] == 18);

        module.clean();
    }
}
