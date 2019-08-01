#include <catch/catch.hpp>

#include <util/config.h>

#include <wasm/WasmModule.h>
#include <util/func.h>

namespace tests {
    void checkPythonFunction(const std::string &funcName) {
        util::SystemConfig &conf = util::getSystemConfig();
        conf.unsafeMode = "on";

        message::Message call = util::messageFactory("python", "py_func");
        call.set_inputdata(funcName);

        wasm::WasmModule module;
        module.initialise();
        module.bindToFunction(call);

        int result = module.execute(call);
        REQUIRE(result == 0);

        conf.reset();
    }

    TEST_CASE("Test python conformance", "[wasm]") {
        checkPythonFunction("lang_test.py");
    }

    TEST_CASE("Test numpy conformance", "[wasm]") {
        checkPythonFunction("numpy_test.py");
    }

    TEST_CASE("Test repeated numpy execution", "[wasm]") {
        util::SystemConfig &conf = util::getSystemConfig();
        conf.unsafeMode = "on";

        message::Message call = util::messageFactory("python", "py_func");
        call.set_inputdata("numpy_test.py");

        wasm::WasmModule module;
        module.initialise();
        module.bindToFunction(call);

        module.snapshotFullMemory("numpy_repeat");
        int result = module.execute(call);
        REQUIRE(result == 0);
        module.restoreFullMemory("numpy_repeat");

        int result1 = module.execute(call);
        REQUIRE(result1 == 0);
        module.restoreFullMemory("numpy_repeat");

        int result2 = module.execute(call);
        REQUIRE(result2 == 0);
        module.restoreFullMemory("numpy_repeat");
    }
}