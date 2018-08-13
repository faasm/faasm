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
        std::vector<uint8_t > expected = {0, 2, 4, 6, 8, 10, 12, 14, 16, 18};
        REQUIRE(outputBytes == expected);

        module.clean();
    }

    TEST_CASE("Test function chaining others", "[worker]") {
        message::FunctionCall call;
        call.set_user("simon");
        call.set_function("chain");

        worker::WasmModule module;

        // Make the call
        int result = module.execute(call);
        REQUIRE(result == 0);

        // Check the chaining
        REQUIRE(3 == module.getChainCount());

        // Check function names
        REQUIRE(module.getChainName(0) == "Function 0");
        REQUIRE(module.getChainName(1) == "Function 1");
        REQUIRE(module.getChainName(2) == "Function 2");

        // Check function data
        std::vector<U8> expected0 = {0, 1, 2};
        std::vector<U8> expected1 = {1, 2, 3};
        std::vector<U8> expected2 = {2, 3, 4};
        REQUIRE(module.getChainData(0) == expected0);
        REQUIRE(module.getChainData(1) == expected1);
        REQUIRE(module.getChainData(2) == expected2);
    }
}
