#include <catch/catch.hpp>
#include <wasm/WasmModule.h>
#include <util/func.h>
#include <util/config.h>
#include <WAVM/Runtime/Intrinsics.h>

using namespace wasm;

namespace tests {
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
              const std::string &inputA,              const std::string &inputB, bool isTypescript) {
        message::Message msgA = util::messageFactory(user, func);

        // Get the initial mem and table size
        Uptr memBeforeA = Runtime::getMemoryNumPages(moduleA.defaultMemory);
        Uptr tableBeforeA;

        if (!isTypescript) {
            tableBeforeA = Runtime::getTableNumElements(moduleA.defaultTable);
        }

        REQUIRE(moduleB.isBound());
        REQUIRE(moduleB.getBoundUser() == moduleA.getBoundUser());
        REQUIRE(moduleB.getBoundFunction() == moduleA.getBoundFunction());
        REQUIRE(moduleB.getBoundIsTypescript() == moduleA.getBoundIsTypescript());

        // Check module B memory and table
        Uptr memBeforeB = Runtime::getMemoryNumPages(moduleB.defaultMemory);
        Uptr tableBeforeB;

        if (!isTypescript) {
            tableBeforeB = Runtime::getTableNumElements(moduleB.defaultTable);
            REQUIRE(tableBeforeB == tableBeforeA);

            // Check tables are different
            REQUIRE(moduleA.defaultTable != moduleB.defaultTable);
        }

        REQUIRE(memBeforeB == memBeforeA);

        // Check important parts are actually different
        REQUIRE(moduleA.defaultMemory != moduleB.defaultMemory);
        REQUIRE(moduleA.compartment != moduleB.compartment);

        // Check base memory addresses are different
        U8 *baseA = Runtime::getMemoryBaseAddress(moduleA.defaultMemory);
        U8 *baseB = Runtime::getMemoryBaseAddress(moduleB.defaultMemory);
        REQUIRE(baseA != baseB);

        // Execute the function with the first module and check it works
        msgA.set_inputdata(inputA);
        int retCodeA = moduleA.execute(msgA);
        REQUIRE(retCodeA == 0);

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

        // Check tables
        Uptr tableAfterA1;
        if (!isTypescript) {
            tableAfterA1 = Runtime::getTableNumElements(moduleA.defaultTable);
            Uptr tableAfterB1 = Runtime::getTableNumElements(moduleB.defaultTable);

            if (func == "py_func") {
                REQUIRE(tableAfterA1 > tableBeforeA);
            } else {
                REQUIRE(tableAfterA1 == tableBeforeA);
            }

            REQUIRE(tableAfterB1 == tableBeforeA);
        }

        // Execute with second module and check
        message::Message msgB = util::messageFactory(user, func);
        msgB.set_inputdata(inputB);
        int retCodeB = moduleB.execute(msgB);
        REQUIRE(retCodeB == 0);

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

            if (func == "py_func") {
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
                               const std::string &inputB, bool isTypescript) {
        message::Message msgA = util::messageFactory(user, func);

        if (isTypescript) {
            msgA.set_istypescript(true);
        }

        WasmModule moduleA;
        moduleA.bindToFunction(msgA);

        WasmModule moduleB(moduleA);

        _doChecks(moduleA, moduleB, user, func, inputA, inputB, isTypescript);
    }

    void _checkAssignmentOperator(const std::string &user, const std::string &func, const std::string &inputA,
                                  const std::string &inputB, bool isTypescript) {
        message::Message msgA = util::messageFactory(user, func);
        msgA.set_istypescript(isTypescript);

        WasmModule moduleA;
        moduleA.bindToFunction(msgA);

        WasmModule moduleB = moduleA;

        _doChecks(moduleA, moduleB, user, func, inputA, inputB, isTypescript);
    }

    TEST_CASE("Test cloned execution on simple module", "[wasm]") {
        std::string user = "demo";
        std::string func = "echo";
        std::string inputA = "aaa";
        std::string inputB = "bbb";

        SECTION("copy") {
            _checkCopyConstructor(user, func, inputA, inputB, false);
        }

        SECTION("assignment") {
            _checkAssignmentOperator(user, func, inputA, inputB, false);
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
        std::string orig = conf.fsMode;
        conf.fsMode = "on";

        std::string user = "python";
        std::string func = "py_func";
        std::string input = "numpy_test";

        SECTION("copy") {
            _checkCopyConstructor(user, func, input, input, false);
        }
        SECTION("assignment") {
            _checkAssignmentOperator(user, func, input, input, false);
        }

        conf.fsMode = orig;
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
