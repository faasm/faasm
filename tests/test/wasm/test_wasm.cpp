#include <catch/catch.hpp>
#include <util/bytes.h>
#include <util/func.h>
#include <util/config.h>
#include <module_cache/WasmModuleCache.h>

using namespace WAVM;

namespace tests {

    TEST_CASE("Test executing WASM module with no input", "[wasm]") {
        message::Message call = util::messageFactory("demo", "dummy");

        wasm::WAVMWasmModule module;
        module.bindToFunction(call);

        // Execute the function
        bool success = module.execute(call);
        REQUIRE(success);

        std::string outputData = call.outputdata();
        const std::vector<uint8_t> outputBytes = util::stringToBytes(outputData);

        // Check output data
        REQUIRE(outputBytes[0] == 0);
        REQUIRE(outputBytes[1] == 1);
        REQUIRE(outputBytes[2] == 2);
        REQUIRE(outputBytes[3] == 3);
    }

    TEST_CASE("Test printf doesn't fail", "[wasm]") {
        message::Message call;
        call.set_user("demo");
        call.set_function("print");

        wasm::WAVMWasmModule module;
        module.bindToFunction(call);

        bool success = module.execute(call);
        REQUIRE(success);
    }

    void executeX2(wasm::WAVMWasmModule &module) {
        message::Message call;
        call.set_user("demo");
        call.set_function("x2");

        // Build input as byte stream
        uint8_t inputValues[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        call.set_inputdata(inputValues, 10);

        // Make the call
        bool success = module.execute(call);
        REQUIRE(success);

        std::string outputData = call.outputdata();
        const std::vector<uint8_t> outputBytes = util::stringToBytes(outputData);

        // Check the results
        std::vector<uint8_t> expected = {0, 2, 4, 6, 8, 10, 12, 14, 16, 18};
        REQUIRE(outputBytes == expected);
    }

    TEST_CASE("Test binding", "[wasm]") {
        message::Message call;
        call.set_user("demo");
        call.set_function("x2");

        wasm::WAVMWasmModule module;
        REQUIRE(!module.isBound());

        module.bindToFunction(call);
        REQUIRE(module.isBound());
    }

    TEST_CASE("Test repeat execution on simple WASM module", "[wasm]") {
        message::Message call;
        call.set_user("demo");
        call.set_function("x2");

        module_cache::WasmModuleCache &registry = module_cache::getWasmModuleCache();
        wasm::WAVMWasmModule &cachedModule = registry.getCachedModule(call);
        
        wasm::WAVMWasmModule module(cachedModule);

        // Perform first execution
        executeX2(module);

        // Reset
        module = cachedModule;

        // Perform repeat executions on same module
        executeX2(module);

        // Reset
        module = cachedModule;

        executeX2(module);
    }

    TEST_CASE("Test execution without binding fails", "[wasm]") {
        message::Message callA;
        callA.set_user("demo");
        callA.set_function("dummy");

        wasm::WAVMWasmModule module;
        REQUIRE_THROWS(module.execute(callA));
    }

    TEST_CASE("Test binding twice fails", "[wasm]") {
        message::Message callA;
        callA.set_user("demo");
        callA.set_function("dummy");

        wasm::WAVMWasmModule module;
        module.bindToFunction(callA);
        REQUIRE_THROWS(module.bindToFunction(callA));
    }

    TEST_CASE("Test repeat execution with different function fails", "[wasm]") {
        message::Message callA = util::messageFactory("demo", "dummy");
        message::Message callB = util::messageFactory("demo", "x2");

        wasm::WAVMWasmModule module;
        module.bindToFunction(callA);

        REQUIRE_THROWS(module.execute(callB));
    }

    TEST_CASE("Test reclaiming memory", "[wasm]") {
        message::Message call = util::messageFactory("demo", "heap");

        module_cache::WasmModuleCache &registry = module_cache::getWasmModuleCache();
        wasm::WAVMWasmModule &cachedModule = registry.getCachedModule(call);
        
        wasm::WAVMWasmModule module(cachedModule);

        Uptr initialPages = Runtime::getMemoryNumPages(module.defaultMemory);

        // Run it (knowing memory will grow during execution)
        module.execute(call);
        
        module = cachedModule;

        Uptr pagesAfter = Runtime::getMemoryNumPages(module.defaultMemory);
        REQUIRE(pagesAfter == initialPages);
    }

    TEST_CASE("Test GC", "[wasm]") {
        wasm::WAVMWasmModule module;
        message::Message call = util::messageFactory("demo", "malloc");

        SECTION("Plain module"){
            // Do nothing
        }
        SECTION("Bound but not executed module"){
            module.bindToFunction(call);
        }
        SECTION("Executed module"){
            module.bindToFunction(call);
            module.execute(call);
        }

        bool success = module.tearDown();
        REQUIRE(success);
    }

    TEST_CASE("Test disassemble module", "[wasm]") {
        message::Message call = util::messageFactory("demo", "echo");
        wasm::WAVMWasmModule module;
        module.bindToFunction(call);

        std::map<std::string, std::string> disasMap = module.buildDisassemblyMap();

        // This may fail because it's too brittle in which case we can do something more flexible

        // Check a few known definitions
        REQUIRE(disasMap["functionDef0"] == "__wasm_call_ctors");
        REQUIRE(disasMap["functionDef1"] == "_start");
        REQUIRE(disasMap["functionDef2"] == "main");
        REQUIRE(disasMap["functionDef3"] == "_faasm_func_0");
        REQUIRE(disasMap["functionDef4"] == "faasmGetInputSize");

        // Check a couple of imports
        REQUIRE(disasMap["functionImport0"] == "__wasi_proc_exit");
        REQUIRE(disasMap["functionImport1"] == "__faasm_read_input");
        REQUIRE(disasMap["functionImport2"] == "__faasm_write_output");
        REQUIRE(disasMap["functionImport3"] == "__faasm_get_idx");
    }
}
