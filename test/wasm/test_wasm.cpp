#include <catch/catch.hpp>
#include <wasm/wasm.h>
#include <util/util.h>

namespace tests {

    TEST_CASE("Test executing WASM module with no input", "[wasm]") {
        message::Message call;
        call.set_user("demo");
        call.set_function("dummy");

        wasm::CallChain callChain(call);

        wasm::WasmModule module;
        module.initialise();

        // Execute the function
        int result = module.execute(call, callChain);
        REQUIRE(result == 0);

        std::string outputData = call.outputdata();
        const std::vector<uint8_t> outputBytes = util::stringToBytes(outputData);

        // Check output data
        REQUIRE(outputBytes[0] == 0);
        REQUIRE(outputBytes[1] == 1);
        REQUIRE(outputBytes[2] == 2);
        REQUIRE(outputBytes[3] == 3);
    }

    TEST_CASE("Test printinf doesn't fail", "[wasm]") {
        message::Message call;
        call.set_user("demo");
        call.set_function("print");

        wasm::CallChain callChain(call);

        wasm::WasmModule module;
        module.initialise();

        int result = module.execute(call, callChain);
        REQUIRE(result == 0);
    }

    void executeX2(wasm::WasmModule &module) {
        message::Message call;
        call.set_user("demo");
        call.set_function("x2");

        wasm::CallChain callChain(call);

        // Build input as byte stream
        U8 inputValues[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        call.set_inputdata(inputValues, 10);

        // Make the call
        int result = module.execute(call, callChain);
        REQUIRE(result == 0);

        std::string outputData = call.outputdata();
        const std::vector<uint8_t> outputBytes = util::stringToBytes(outputData);

        // Check the results
        std::vector<uint8_t> expected = {0, 2, 4, 6, 8, 10, 12, 14, 16, 18};
        REQUIRE(outputBytes == expected);
    }

    TEST_CASE("Test executing WASM module with input and output", "[wasm]") {
        wasm::WasmModule module;
        module.initialise();

        // Perform first execution
        executeX2(module);

        // Perform repeat executions on same module
        executeX2(module);
        executeX2(module);
    }

    TEST_CASE("Test repeat execution with different function fails", "[wasm]") {
        message::Message callA;
        callA.set_user("demo");
        callA.set_function("dummy");
        wasm::CallChain callChainA(callA);

        message::Message callB;
        callB.set_user("demo");
        callB.set_function("x2");
        wasm::CallChain callChainB(callB);

        wasm::WasmModule module;
        module.initialise();
        module.execute(callA, callChainA);

        REQUIRE_THROWS(module.execute(callB, callChainB));
    }
}
