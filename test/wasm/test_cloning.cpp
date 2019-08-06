#include <catch/catch.hpp>
#include <wasm/WasmModule.h>
#include <util/func.h>
#include <util/config.h>

namespace tests {
    TEST_CASE("Test cloning empty modules doesn't break", "[wasm]") {
        wasm::WasmModule moduleA;
        wasm::WasmModule moduleB(moduleA);
    }

    TEST_CASE("Test cloning plain modules doesn't break", "[wasm]") {
        wasm::WasmModule moduleA;

        wasm::WasmModule moduleB(moduleA);
    }

    void _checkCloning(const std::string &user, const std::string &func, const std::string &inputA, const std::string &inputB) {
        message::Message msgA = util::messageFactory(user, func);

        // Create and bind one module
        wasm::WasmModule moduleA;
        moduleA.bindToFunction(msgA);

        // Get the initial mem and table size
        Uptr memBeforeA = Runtime::getMemoryNumPages(moduleA.defaultMemory);
        Uptr tableBeforeA = Runtime::getTableNumElements(moduleA.defaultTable);

        wasm::WasmModule moduleB(moduleA);
        REQUIRE(moduleB.isBound());
        REQUIRE(moduleB.getBoundUser() == moduleA.getBoundUser());
        REQUIRE(moduleB.getBoundFunction() == moduleA.getBoundFunction());

        // Check module B memory and table
        Uptr memBeforeB = Runtime::getMemoryNumPages(moduleB.defaultMemory);
        Uptr tableBeforeB = Runtime::getTableNumElements(moduleB.defaultTable);
        REQUIRE(memBeforeB == memBeforeA);
        REQUIRE(tableBeforeB == tableBeforeA);

        // Check important parts are actually different
        REQUIRE(moduleA.defaultMemory != moduleB.defaultMemory);
        REQUIRE(moduleA.defaultTable != moduleB.defaultTable);
        REQUIRE(moduleA.compartment != moduleB.compartment);

        // Check base memory addresses are different
        U8 *baseA = Runtime::getMemoryBaseAddress(moduleA.defaultMemory);
        U8 *baseB = Runtime::getMemoryBaseAddress(moduleB.defaultMemory);
        REQUIRE(baseA != baseB);

        // Execute the function with the first module and check it works
        msgA.set_inputdata(inputA);
        int retCodeA = moduleA.execute(msgA);
        REQUIRE(retCodeA == 0);

        if(func == "echo") {
            REQUIRE(msgA.outputdata() == inputA);
        }

        // Check memory sizes
        Uptr memAfterA1 = Runtime::getMemoryNumPages(moduleA.defaultMemory);
        Uptr memAfterB1 = Runtime::getMemoryNumPages(moduleB.defaultMemory);
        REQUIRE(memAfterB1 == memBeforeA);
        REQUIRE(memAfterA1 == memBeforeA);

        Uptr tableAfterA1 = Runtime::getTableNumElements(moduleA.defaultTable);
        Uptr tableAfterB1 = Runtime::getTableNumElements(moduleB.defaultTable);
        REQUIRE(tableAfterB1 == tableBeforeA);
        REQUIRE(tableAfterA1 == tableBeforeA);

        // Execute with second module and check
        message::Message msgB = util::messageFactory(user, func);
        msgB.set_inputdata(inputB);
        int retCodeB = moduleB.execute(msgB);
        REQUIRE(retCodeB == 0);

        if(func == "echo") {
            REQUIRE(msgB.outputdata() == inputB);
        }

        // Check memory sizes
        Uptr memAfterA2 = Runtime::getMemoryNumPages(moduleA.defaultMemory);
        Uptr memAfterB2 = Runtime::getMemoryNumPages(moduleB.defaultMemory);
        REQUIRE(memAfterA1 == memAfterA2);
        REQUIRE(memAfterB2 == memAfterA2);
        REQUIRE(memAfterB2 == memBeforeB);

        Uptr tableAfterA2 = Runtime::getTableNumElements(moduleA.defaultTable);
        Uptr tableAfterB2 = Runtime::getTableNumElements(moduleB.defaultTable);
        REQUIRE(tableAfterA1 == tableAfterA2);
        REQUIRE(tableAfterB2 == tableAfterA2);
        REQUIRE(tableAfterB2 == tableBeforeB);
    }

    TEST_CASE("Test cloned execution on simple module", "[wasm]") {
        std::string user = "demo";
        std::string func = "echo";
        std::string inputA = "aaa";
        std::string inputB = "bbb";

        _checkCloning(user, func, inputA, inputB);
    }

    TEST_CASE("Test cloned execution on complex module", "[wasm]") {
        util::SystemConfig &conf = util::getSystemConfig();
        std::string orig = conf.unsafeMode;
        conf.unsafeMode = "on";

        std::string user = "python";
        std::string func = "py_func";
        std::string input = "numpy_test.py";

        _checkCloning(user, func, input, input);

        conf.unsafeMode = orig;
    }

}
