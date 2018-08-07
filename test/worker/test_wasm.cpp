#include <catch/catch.hpp>
#include <worker/worker.h>

namespace tests {

    TEST_CASE("Test executing WASM module with no input", "[worker]") {
        message::FunctionCall call;
        call.set_user("simon");
        call.set_function("dummy");

        worker::WasmModule module;

        // Execute the function
        std::vector<U8> inputData;
        int result = module.execute(call, inputData);
        REQUIRE(result == 0);

        // Check output data
        std::vector<I32> outputData = module.getOutputData();
        REQUIRE(outputData[0] == 0);
        REQUIRE(outputData[1] == 1);
        REQUIRE(outputData[2] == 2);
        REQUIRE(outputData[3] == 3);

        module.clean();
    }

    TEST_CASE("Test executing WASM module with input and output", "[worker]") {
        message::FunctionCall call;
        call.set_user("simon");
        call.set_function("x2");

        worker::WasmModule module;

        // Build input as byte stream
        int inputValues[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

        U8* bytesPointer = reinterpret_cast<U8*>(inputValues);
        int nBytes = 40;

        std::vector<U8> inputBytes(bytesPointer, bytesPointer + nBytes);

        // Make the call
        int result = module.execute(call, inputBytes);
        REQUIRE(result == 0);

        // Check the results
        std::vector<I32> outputData = module.getOutputData();
        REQUIRE(outputData[0] == 0);
        REQUIRE(outputData[1] == 1);
        REQUIRE(outputData[2] == 2);
        REQUIRE(outputData[3] == 3);

        module.clean();
    }
}
