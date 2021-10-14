#include "utils.h"
#include <catch2/catch.hpp>

#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/func.h>

#include <wavm/WAVMWasmModule.h>

using namespace WAVM;

namespace tests {

TEST_CASE("Test executing WASM module with no input", "[wasm]")
{
    cleanSystem();

    faabric::Message call = faabric::util::messageFactory("demo", "dummy");

    wasm::WAVMWasmModule module;
    module.bindToFunction(call);

    // Execute the function
    int returnValue = module.executeFunction(call);
    REQUIRE(returnValue == 0);

    std::string outputData = call.outputdata();
    const std::vector<uint8_t> outputBytes =
      faabric::util::stringToBytes(outputData);

    // Check output data
    REQUIRE(outputBytes[0] == 0);
    REQUIRE(outputBytes[1] == 1);
    REQUIRE(outputBytes[2] == 2);
    REQUIRE(outputBytes[3] == 3);
}

TEST_CASE("Test printf doesn't fail", "[wasm]")
{
    cleanSystem();

    faabric::Message call;
    call.set_user("demo");
    call.set_function("print");

    wasm::WAVMWasmModule module;
    module.bindToFunction(call);

    int returnValue = module.executeFunction(call);
    REQUIRE(returnValue == 0);
}

void executeX2(wasm::WAVMWasmModule& module)
{
    faabric::Message call;
    call.set_user("demo");
    call.set_function("x2");

    // Build input as byte stream
    uint8_t inputValues[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    call.set_inputdata(inputValues, 10);

    // Make the call
    int returnValue = module.executeFunction(call);
    REQUIRE(returnValue == 0);

    std::string outputData = call.outputdata();
    const std::vector<uint8_t> outputBytes =
      faabric::util::stringToBytes(outputData);

    // Check the results
    std::vector<uint8_t> expected = { 0, 2, 4, 6, 8, 10, 12, 14, 16, 18 };
    REQUIRE(outputBytes == expected);
}

TEST_CASE("Test binding", "[wasm]")
{
    cleanSystem();

    faabric::Message call;
    call.set_user("demo");
    call.set_function("x2");

    wasm::WAVMWasmModule module;
    REQUIRE(!module.isBound());

    module.bindToFunction(call);
    REQUIRE(module.isBound());
}

TEST_CASE("Test repeat execution on simple WASM module", "[wasm]")
{
    cleanSystem();

    faabric::Message call;
    call.set_user("demo");
    call.set_function("x2");

    wasm::WAVMWasmModule module;
    module.bindToFunction(call);

    // Perform first execution
    executeX2(module);

    // Reset
    module.reset(call, "");

    // Perform repeat executions on same module
    executeX2(module);

    // Reset
    module.reset(call, "");

    executeX2(module);
}

TEST_CASE("Test execution without binding fails", "[wasm]")
{
    cleanSystem();

    faabric::Message callA;
    callA.set_user("demo");
    callA.set_function("dummy");

    wasm::WAVMWasmModule module;
    REQUIRE_THROWS(module.executeFunction(callA));
}

TEST_CASE("Test reclaiming memory", "[wasm]")
{
    cleanSystem();

    faabric::Message call = faabric::util::messageFactory("demo", "heap");

    wasm::WAVMWasmModule module;
    module.bindToFunction(call);

    Uptr initialPages = Runtime::getMemoryNumPages(module.defaultMemory);

    // Run it (knowing memory will grow during execution)
    module.executeFunction(call);

    module.reset(call, "");

    Uptr pagesAfter = Runtime::getMemoryNumPages(module.defaultMemory);
    REQUIRE(pagesAfter == initialPages);
}

TEST_CASE("Test disassemble module", "[wasm]")
{
    cleanSystem();

    faabric::Message call = faabric::util::messageFactory("demo", "echo");
    wasm::WAVMWasmModule module;
    module.bindToFunction(call);

    std::map<std::string, std::string> disasMap = module.buildDisassemblyMap();

    // Note: this test assumes the compilation output is always the same. We
    // could change it to search through definitions rather than assume their
    // positions.

    // Check a few known definitions
    REQUIRE(disasMap["functionDef0"] == "__wasm_call_ctors");
    REQUIRE(disasMap["functionDef1"] == "_start");
    REQUIRE(disasMap["functionDef2"] == "main");
    REQUIRE(disasMap["functionDef3"] == "faasmGetInputSize");
    REQUIRE(disasMap["functionDef4"] == "faasmGetInput");

    // Check a couple of imports
    REQUIRE(disasMap["functionImport0"] == "__wasi_proc_exit");
    REQUIRE(disasMap["functionImport1"] == "__faasm_read_input");
    REQUIRE(disasMap["functionImport2"] == "__faasm_write_output");
    REQUIRE(disasMap["functionImport3"] == "pthread_mutex_lock");
}
}
