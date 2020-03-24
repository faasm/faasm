#include <catch/catch.hpp>
#include <wasm/WasmModule.h>
#include <util/func.h>
#include <util/config.h>
#include <WAVM/Runtime/Intrinsics.h>
#include <utils.h>

using namespace wasm;

namespace tests {
    void convertMsgToPython(message::Message &msg) {
        msg.set_pythonuser(msg.user());
        msg.set_pythonfunction(msg.function());
        msg.set_user(PYTHON_USER);
        msg.set_function(PYTHON_FUNC);
        msg.set_ispython(true);
    }

    TEST_CASE("Test cloning empty modules doesn't break", "[wasm]") {
        WasmModule moduleA;
        WasmModule moduleB(moduleA);

        WasmModule moduleC;
        moduleC = moduleA;

        // Check clear-up
        REQUIRE(moduleA.tearDown());
        REQUIRE(moduleB.tearDown());
        REQUIRE(moduleC.tearDown());
    }

    void
    _doChecks(wasm::WasmModule &moduleA, wasm::WasmModule &moduleB, const std::string &user, const std::string &func,
              const std::string &inputA,              const std::string &inputB, bool isTypescript, bool isPython) {

        message::Message msgA = util::messageFactory(user, func);
        message::Message msgB = util::messageFactory(user, func);

        msgA.set_inputdata(inputA);
        msgB.set_inputdata(inputB);

        if(isPython) {
            convertMsgToPython(msgA);
            convertMsgToPython(msgB);
        }

        // Dummy execution initially to avoid any first-time set-up differences
        WasmModule moduleWarmUp;
        moduleWarmUp.bindToFunction(msgA);
        moduleWarmUp.execute(msgA);

        // Get the initial mem and table size
        Uptr memBeforeA = Runtime::getMemoryNumPages(moduleA.defaultMemory);
        Uptr memBeforeB = Runtime::getMemoryNumPages(moduleB.defaultMemory);

        Uptr tableBeforeB;
        Uptr tableBeforeA;
        if (!isTypescript) {
            tableBeforeA = Runtime::getTableNumElements(moduleA.defaultTable);
            tableBeforeB = Runtime::getTableNumElements(moduleB.defaultTable);

            REQUIRE(tableBeforeB == tableBeforeA);

            // Check tables are different
            REQUIRE(moduleA.defaultTable != moduleB.defaultTable);
        }

        // Check basic properties
        REQUIRE(moduleB.isBound());
        REQUIRE(moduleB.getBoundUser() == moduleA.getBoundUser());
        REQUIRE(moduleB.getBoundFunction() == moduleA.getBoundFunction());
        REQUIRE(moduleB.getBoundIsTypescript() == moduleA.getBoundIsTypescript());

        REQUIRE(memBeforeB == memBeforeA);

        // Check important parts are actually different
        REQUIRE(moduleA.defaultMemory != moduleB.defaultMemory);
        REQUIRE(moduleA.compartment != moduleB.compartment);

        // Check base memory addresses are different
        U8 *baseA = Runtime::getMemoryBaseAddress(moduleA.defaultMemory);
        U8 *baseB = Runtime::getMemoryBaseAddress(moduleB.defaultMemory);
        REQUIRE(baseA != baseB);

        // Execute the function with the first module and check it works
        bool successA = moduleA.execute(msgA);
        REQUIRE(successA);

        if (func == "echo") {
            REQUIRE(msgA.outputdata() == inputA);
        }

        // Check memory has grown in the one that's executed (unless it's typescript)
        Uptr memAfterA1 = Runtime::getMemoryNumPages(moduleA.defaultMemory);
        Uptr memAfterB1 = Runtime::getMemoryNumPages(moduleB.defaultMemory);
        REQUIRE(memAfterB1 == memBeforeA);

        if(isTypescript) {
            REQUIRE(memAfterA1 == memBeforeA);
        } else {
            REQUIRE(memAfterA1 > memBeforeA);
        }

        // Check tables (should have grown for Python and not for other)
        Uptr tableAfterA1;
        if (!isTypescript) {
            tableAfterA1 = Runtime::getTableNumElements(moduleA.defaultTable);
            Uptr tableAfterB1 = Runtime::getTableNumElements(moduleB.defaultTable);

            if(isPython) {
                REQUIRE(tableAfterA1 > tableBeforeA);
            } else {
                REQUIRE(tableAfterA1 == tableBeforeA);
            }

            REQUIRE(tableAfterB1 == tableBeforeA);
        }

        // Execute with second module
        bool successB = moduleB.execute(msgB);
        REQUIRE(successB);

        if (func == "echo") {
            REQUIRE(msgB.outputdata() == inputB);
        }

        // Check memory sizes
        Uptr memAfterA2 = Runtime::getMemoryNumPages(moduleA.defaultMemory);
        Uptr memAfterB2 = Runtime::getMemoryNumPages(moduleB.defaultMemory);
        REQUIRE(memAfterB2 == memAfterA2);
        REQUIRE(memAfterA1 == memAfterA2);

        if(isTypescript) {
            REQUIRE(memAfterB2 == memBeforeB);
        } else {
            REQUIRE(memAfterB2 > memBeforeB);

            Uptr tableAfterA2 = Runtime::getTableNumElements(moduleA.defaultTable);
            Uptr tableAfterB2 = Runtime::getTableNumElements(moduleB.defaultTable);

            if(isPython) {
                REQUIRE(tableAfterB2 > tableBeforeB);
            } else {
                REQUIRE(tableAfterB2 == tableBeforeB);
            }

            REQUIRE(tableAfterB2 == tableAfterA2);
            REQUIRE(tableAfterA1 == tableAfterA2);
        }

        // Check successful clean-up
        REQUIRE(moduleA.tearDown());
        REQUIRE(moduleB.tearDown());
    }

    void _checkCopyConstructor(const std::string &user, const std::string &func, const std::string &inputA,
                               const std::string &inputB, bool isTypescript, bool isPython) {
        message::Message msgA = util::messageFactory(user, func);

        if(isPython) {
            convertMsgToPython(msgA);
        }

        if (isTypescript) {
            msgA.set_istypescript(true);
        }

        WasmModule moduleA;
        moduleA.bindToFunction(msgA);

        WasmModule moduleB(moduleA);
        _doChecks(moduleA, moduleB, user, func, inputA, inputB, isTypescript, isPython);
    }

    void _checkAssignmentOperator(const std::string &user, const std::string &func, const std::string &inputA,
                                  const std::string &inputB, bool isTypescript, bool isPython) {
        message::Message msgA = util::messageFactory(user, func);
        msgA.set_istypescript(isTypescript);

        if(isPython) {
            convertMsgToPython(msgA);
        }

        WasmModule moduleA;
        moduleA.bindToFunction(msgA);

        WasmModule moduleB = moduleA;
        _doChecks(moduleA, moduleB, user, func, inputA, inputB, isTypescript, isPython);
    }

    TEST_CASE("Test cloned execution on simple module", "[wasm]") {
        std::string user = "demo";
        std::string func = "echo";
        std::string inputA = "aaa";
        std::string inputB = "bbb";

        SECTION("copy") {
            _checkCopyConstructor(user, func, inputA, inputB, false, false);
        }

        SECTION("assignment") {
            _checkAssignmentOperator(user, func, inputA, inputB, false, false);
        }
    }

    // TODO - fix typescript support
//    TEST_CASE("Test cloned execution on typescript module", "[wasm]") {
//        std::string user = "ts";
//        std::string func = "echo";
//        std::string inputA = "aaa";
//        std::string inputB = "bbb";
//
//        SECTION("copy") {
//            _checkCopyConstructor(user, func, inputA, inputB, true);
//        }
//
//        SECTION("assignment") {
//            _checkAssignmentOperator(user, func, inputA, inputB, true);
//        }
//    }

    TEST_CASE("Test cloned execution on complex module", "[wasm]") {
        util::SystemConfig &conf = util::getSystemConfig();
        std::string preloadVal = conf.pythonPreload;
        conf.pythonPreload = "off";
        
        std::string user = "python";
        std::string func = "numpy_test";
        std::string input;

        SECTION("copy") {
            _checkCopyConstructor(user, func, input, input, false, true);
        }
        SECTION("assignment") {
            _checkAssignmentOperator(user, func, input, input, false, true);
        }

        conf.pythonPreload = preloadVal;
    }

    TEST_CASE("Test GC on cloned modules without execution") {
        message::Message msg = util::messageFactory("demo", "echo");

        WasmModule moduleA;
        moduleA.bindToFunction(msg);

        WasmModule moduleB(moduleA);

        WasmModule moduleC = moduleA;

        REQUIRE(moduleA.tearDown());
        REQUIRE(moduleB.tearDown());
        REQUIRE(moduleC.tearDown());
    }

    TEST_CASE("Test GC on cloned modules with execution") {
        message::Message msg = util::messageFactory("demo", "echo");

        WasmModule moduleA;
        moduleA.bindToFunction(msg);
        moduleA.execute(msg);

        WasmModule moduleB(moduleA);
        moduleB.execute(msg);

        WasmModule moduleC = moduleA;
        moduleC.execute(msg);

        REQUIRE(moduleA.tearDown());
        REQUIRE(moduleB.tearDown());
        REQUIRE(moduleC.tearDown());
    }
}
