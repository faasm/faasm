#include <catch/catch.hpp>
#include <wamr/WAMRWasmModule.h>
#include <util/func.h>
#include <util/config.h>

using namespace wasm;

namespace tests {
    TEST_CASE("Test executing a function with WAMR doesn't break", "[wasm]") {
        message::Message call = util::messageFactory("demo", "hello");

        wasm::WAMRWasmModule module;
        module.bindToFunction(call);
        module.execute(call);
    }
}
