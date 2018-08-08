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
        REQUIRE(module.outputData[0] == 0);
        REQUIRE(module.outputData[1] == 1);
        REQUIRE(module.outputData[2] == 2);
        REQUIRE(module.outputData[3] == 3);

        module.clean();
    }

//    TEST_CASE("Test executing WASM module with input and output", "[worker]") {
//        message::FunctionCall call;
//        call.set_user("simon");
//        call.set_function("x2");
//
//        worker::WasmModule module;
//
//        // Build input as byte stream
//        std::vector<U8> inputValues = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
//
//        // Make the call
//        int result = module.execute(call, inputValues);
//        REQUIRE(result == 0);
//
//        // Check the results
//        REQUIRE(module.outputData[0] == 0);
//        REQUIRE(module.outputData[1] == 2);
//        REQUIRE(module.outputData[2] == 4);
//        REQUIRE(module.outputData[3] == 6);
//        REQUIRE(module.outputData[4] == 8);
//        REQUIRE(module.outputData[5] == 10);
//        REQUIRE(module.outputData[6] == 12);
//        REQUIRE(module.outputData[7] == 14);
//        REQUIRE(module.outputData[8] == 16);
//        REQUIRE(module.outputData[9] == 18);
//
//        module.clean();
//    }
}
