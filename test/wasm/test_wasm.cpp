#include <catch/catch.hpp>
#include <wasm/wasm.h>
#include <util/util.h>

namespace tests {

    TEST_CASE("Test executing WASM module with no input", "[wasm]") {
        message::FunctionCall call;
        call.set_user("simon");
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
        call.set_user("simon");
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

    TEST_CASE("Test function chaining others", "[wasm]") {
        message::FunctionCall call;
        call.set_user("simon");
        call.set_function("chain");

        wasm::WasmModule module;

        // Make the call
        int result = module.execute(call);
        REQUIRE(result == 0);

        // Check the chaining
        REQUIRE(3 == module.chainedCalls.size());

        // Check all are set with the right user
        REQUIRE(module.chainedCalls[0].user() == "simon");
        REQUIRE(module.chainedCalls[1].user() == "simon");
        REQUIRE(module.chainedCalls[2].user() == "simon");

        // Check function names
        REQUIRE(module.chainedCalls[0].function() == "Function 0");
        REQUIRE(module.chainedCalls[1].function() == "Function 1");
        REQUIRE(module.chainedCalls[2].function() == "Function 2");

        // Check function data
        std::vector<uint8_t> expected0 = {0, 1, 2};
        std::vector<uint8_t> expected1 = {1, 2, 3};
        std::vector<uint8_t> expected2 = {2, 3, 4};
        REQUIRE(util::stringToBytes(module.chainedCalls[0].inputdata()) == expected0);
        REQUIRE(util::stringToBytes(module.chainedCalls[1].inputdata()) == expected1);
        REQUIRE(util::stringToBytes(module.chainedCalls[2].inputdata()) == expected2);
    }
}
