#include <catch/catch.hpp>

#include <util/config.h>

#include <wasm/WasmModule.h>
#include <util/func.h>
#include <utils.h>

namespace tests {
    void checkPythonFunction(const std::string &funcName) {
        util::SystemConfig &conf = util::getSystemConfig();
        conf.fsMode = "on";

        message::Message call = util::messageFactory("python", funcName);
        util::convertMessageToPython(call);

        execFunction(call);

        conf.reset();
    }

    TEST_CASE("Test python conformance", "[worker]") {
        checkPythonFunction("lang_test");
    }

    TEST_CASE("Test numpy conformance", "[worker]") {
        checkPythonFunction("numpy_test");
    }

    TEST_CASE("Test repeated numpy execution", "[worker]") {
        util::SystemConfig &conf = util::getSystemConfig();
        conf.fsMode = "on";

        message::Message call = util::messageFactory("python", "numpy_test");
        util::convertMessageToPython(call);

        checkMultipleExecutions(call, 3);
    }
}