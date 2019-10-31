#include <catch/catch.hpp>
#include <wasm/WasmModule.h>
#include <util/bytes.h>
#include <util/func.h>
#include <util/config.h>
#include <utils.h>


namespace tests {
    TEST_CASE("Test executing tensorflow lite model", "[wasm]") {
        message::Message call = util::messageFactory("tf", "image");
        execFunction(call);
    }
}