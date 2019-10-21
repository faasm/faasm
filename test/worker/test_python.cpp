#include <catch/catch.hpp>

#include <util/config.h>

#include <wasm/WasmModule.h>
#include <util/func.h>
#include <utils.h>

namespace tests {
    void checkPythonFunction(const std::string &funcName) {
        cleanSystem();

        util::SystemConfig &conf = util::getSystemConfig();
        conf.fsMode = "on";

        message::Message call = util::messageFactory(PYTHON_USER, PYTHON_FUNC);
        call.set_pythonuser("python");
        call.set_pythonfunction(funcName);
        call.set_ispython(true);

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

        message::Message call = util::messageFactory(PYTHON_USER, PYTHON_FUNC);
        call.set_pythonuser("python");
        call.set_pythonfunction("numpy_test");
        call.set_ispython(true);

        checkMultipleExecutions(call, 3);
    }
}