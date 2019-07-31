#include <catch/catch.hpp>

#include <util/config.h>

#include <wasm/WasmModule.h>
#include <util/func.h>

namespace tests {
    TEST_CASE("Test python conformance", "[wasm]") {
        util::SystemConfig &conf = util::getSystemConfig();
        conf.unsafeMode = "on";

        std::string funcName;
        SECTION("Standard language features") {
            funcName = "lang_test.py";
        }

        SECTION("Numpy features") {
            funcName = "numpy_test.py";
        }

        message::Message call = util::messageFactory("python", "py_func");
        call.set_inputdata(funcName);

        wasm::WasmModule module;
        module.initialise();
        module.bindToFunction(call);

        int result = module.execute(call);
        REQUIRE(result == 0);

        conf.reset();
    }
}