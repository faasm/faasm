#include <catch/catch.hpp>
#include <worker/worker.h>

namespace tests {

    TEST_CASE("Test executing simple WASM module", "[worker]") {
        // Note, we require this function to be present
        message::FunctionCall call;
        call.set_user("simon");
        call.set_function("dummy");

        worker::WasmModule module;

        // Execute the function
        int result = module.execute(call);
        REQUIRE(result == 0);

        std::vector<U8> outputData = module.getOutputData();
        REQUIRE(outputData[0] == 0);
        REQUIRE(outputData[1] == 1);
        REQUIRE(outputData[2] == 2);
        REQUIRE(outputData[3] == 3);

        module.clean();
    }
}
