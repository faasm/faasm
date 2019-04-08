#include <catch/catch.hpp>

#include <util/config.h>

#include <wasm/WasmModule.h>

namespace tests {
    TEST_CASE("Test executing a python wasm function", "[wasm]") {
        util::SystemConfig &conf = util::getSystemConfig();
        conf.unsafeMode = "on";

        message::Message call;
        call.set_user("python");
        call.set_function("py_hello");

        wasm::CallChain callChain(call);

        wasm::WasmModule module;
        module.initialise();
        module.bindToFunction(call);

        int result = module.execute(call, callChain);
        REQUIRE(result == 0);

        conf.reset();
    }
}