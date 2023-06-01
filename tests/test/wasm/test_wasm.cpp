#include <catch2/catch.hpp>

#include "faabric_utils.h"
#include "faasm_fixtures.h"
#include "utils.h"

#include <faabric/util/bytes.h>
#include <faabric/util/config.h>
#include <faabric/util/func.h>

#include <wavm/WAVMWasmModule.h>

using namespace WAVM;

namespace tests {

class SimpleWasmTestFixture : public FunctionExecTestFixture
{
  public:
    void executeX2(wasm::WAVMWasmModule& module)
    {
        auto req = setUpContext("demo", "x2");
        faabric::Message& msg = req->mutable_messages()->at(0);

        // Build input as byte stream
        uint8_t inputValues[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        msg.set_inputdata(inputValues, 10);

        // Make the call
        int returnValue = module.executeFunction(msg);
        REQUIRE(returnValue == 0);

        std::string output = msg.outputdata();
        std::string expected = "success";

        REQUIRE(output == expected);
    }
};

TEST_CASE_METHOD(SimpleWasmTestFixture,
                 "Test executing WASM module with no input",
                 "[wasm]")
{
    auto req = setUpContext("demo", "dummy");

    wasm::WAVMWasmModule module;
    faabric::Message& msg = req->mutable_messages()->at(0);
    module.bindToFunction(msg);

    // Execute the function
    int returnValue = module.executeFunction(msg);
    REQUIRE(returnValue == 0);

    std::string outputData = msg.outputdata();

    // Check output data
    REQUIRE(outputData == "dummy");
}

TEST_CASE_METHOD(SimpleWasmTestFixture, "Test printf doesn't fail", "[wasm]")
{
    auto req = setUpContext("demo", "print");
    faabric::Message& msg = req->mutable_messages()->at(0);

    wasm::WAVMWasmModule module;
    module.bindToFunction(msg);
    int returnValue = module.executeFunction(msg);
    REQUIRE(returnValue == 0);
}

TEST_CASE_METHOD(SimpleWasmTestFixture, "Test binding", "[wasm]")
{
    faabric::Message msg = faabric::util::messageFactory("demo", "x2");
    wasm::WAVMWasmModule module;
    REQUIRE(!module.isBound());

    module.bindToFunction(msg);
    REQUIRE(module.isBound());
}

TEST_CASE_METHOD(SimpleWasmTestFixture,
                 "Test repeat execution on simple WASM module",
                 "[wasm]")
{
    faabric::Message msg = faabric::util::messageFactory("demo", "x2");
    wasm::WAVMWasmModule module;
    module.bindToFunction(msg);

    // Perform first execution
    executeX2(module);

    // Reset
    module.reset(msg, "");

    // Perform repeat executions on same module
    executeX2(module);

    // Reset
    module.reset(msg, "");

    executeX2(module);
}

TEST_CASE_METHOD(SimpleWasmTestFixture,
                 "Test execution without binding fails",
                 "[wasm]")
{
    faabric::Message msg = faabric::util::messageFactory("demo", "dummy");
    wasm::WAVMWasmModule module;
    REQUIRE_THROWS(module.executeFunction(msg));
}

TEST_CASE_METHOD(SimpleWasmTestFixture, "Test reclaiming memory", "[wasm]")
{
    auto req = setUpContext("demo", "heap");
    faabric::Message& call = req->mutable_messages()->at(0);
    wasm::WAVMWasmModule module;
    module.bindToFunction(call);

    Uptr initialPages = Runtime::getMemoryNumPages(module.defaultMemory);

    // Run it (knowing memory will grow during execution)
    module.executeFunction(call);

    module.reset(call, "");

    Uptr pagesAfter = Runtime::getMemoryNumPages(module.defaultMemory);
    REQUIRE(pagesAfter == initialPages);
}

TEST_CASE_METHOD(SimpleWasmTestFixture, "Test disassemble module", "[wasm]")
{
    auto req = setUpContext("demo", "echo");
    faabric::Message& call = req->mutable_messages()->at(0);
    wasm::WAVMWasmModule module;
    module.bindToFunction(call);

    std::map<std::string, std::string> disasMap = module.buildDisassemblyMap();

    // Note: this test assumes the compilation output is always the same. We
    // could change it to search through definitions rather than assume their
    // positions.

    // Check a few known definitions
    REQUIRE(disasMap["functionDef0"] == "__wasm_call_ctors");
    REQUIRE(disasMap["functionDef1"] == "_start");
    REQUIRE(disasMap["functionDef2"] == "__faasm_memory_layout_protection()");
    REQUIRE(disasMap["functionDef3"] == "main");
    REQUIRE(disasMap["functionDef4"] == "faasmGetInputSize");

    // Check a couple of imports
    REQUIRE(disasMap["functionImport0"] == "__faasm_read_input");
    REQUIRE(disasMap["functionImport1"] == "__faasm_write_output");
    REQUIRE(disasMap["functionImport2"] ==
            "__imported_wasi_snapshot_preview1_args_get");
    REQUIRE(disasMap["functionImport3"] ==
            "__imported_wasi_snapshot_preview1_args_sizes_get");
}
}
