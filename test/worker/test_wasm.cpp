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
        module.execute(call);
        const char *result = module.resultToCharPtr();

        REQUIRE(strcmp("THIS IS MY DATA", result) == 0);

        module.clean();
    }
}
