#include <catch/catch.hpp>

#include <wasm/WasmModule.h>

namespace tests {
    TEST_CASE("Placeholder", "[wasm]") {
        // Here until Python stuff works
        REQUIRE(true);
    }

//    TEST_CASE("Test executing a python wasm function", "[wasm]") {
//        message::Message call;
//        call.set_user("python");
//        call.set_function("py_hello");
//
//        wasm::CallChain callChain(call);
//
//        wasm::WasmModule module;
//        module.initialise();
//        module.bindToFunction(call);
//
//        int result = module.execute(call, callChain);
//        REQUIRE(result == 0);
//    }
}