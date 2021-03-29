#include <catch2/catch.hpp>

#include "utils.h"

#include <boost/filesystem.hpp>
#include <faabric/util/func.h>
#include <wavm/WAVMWasmModule.h>

using namespace wasm;

namespace tests {

TEST_CASE("Test snapshot and restore for wasm module", "[wasm]")
{
    cleanSystem();

    std::string user = "demo";
    std::string function = "zygote_check";
    faabric::Message m = faabric::util::messageFactory(user, function);

    std::string mode;
    SECTION("In memory") { mode = "memory"; }

    SECTION("In file") { mode = "file"; }

    SECTION("In state") { mode = "state"; }

    std::vector<uint8_t> memoryData;

    std::string stateKey = "serialTest";
    size_t stateSize;

    // Prepare output file
    std::string filePath = "/tmp/faasm_serialised";
    if (boost::filesystem::exists(filePath.c_str())) {
        boost::filesystem::remove(filePath.c_str());
    }

    // Create the full module
    wasm::WAVMWasmModule moduleA;
    moduleA.bindToFunction(m);

    // Modify the memory of the module to check changes are propagated
    uint32_t wasmPtr = moduleA.mmapMemory(1);
    uint8_t* nativePtr = moduleA.wasmPointerToNative(wasmPtr);
    nativePtr[0] = 0;
    nativePtr[1] = 1;
    nativePtr[2] = 2;
    nativePtr[3] = 3;
    nativePtr[4] = 4;

    // Add some guard regions to make sure these can be propagated
    moduleA.createMemoryGuardRegion();
    moduleA.createMemoryGuardRegion();

    size_t expectedSizeBytes = moduleA.getMemorySizeBytes();

    if (mode == "memory") {
        // Serialise to memory
        memoryData = moduleA.snapshotToMemory();
    } else if (mode == "file") {
        // Serialise to file
        moduleA.snapshotToFile(filePath);
    } else {
        // Serialise to state
        stateSize = moduleA.snapshotToState(stateKey);
    }

    // Create the module to be restored but don't execute zygote
    wasm::WAVMWasmModule moduleB;
    moduleB.bindToFunctionNoZygote(m);

    // Restore from snapshot
    if (mode == "memory") {
        moduleB.restoreFromMemory(memoryData);
    } else if (mode == "file") {
        moduleB.restoreFromFile(filePath);
    } else {
        moduleB.restoreFromState(stateKey, stateSize);
    }

    // Check size of restored memory is as expected
    size_t actualSizeBytesB = moduleB.getMemorySizeBytes();
    REQUIRE(actualSizeBytesB == expectedSizeBytes);

    // Check writes to memory are visible in restored module
    uint8_t* nativePtrB = moduleB.wasmPointerToNative(wasmPtr);
    REQUIRE(nativePtrB[0] == 0);
    REQUIRE(nativePtrB[1] == 1);
    REQUIRE(nativePtrB[2] == 2);
    REQUIRE(nativePtrB[3] == 3);
    REQUIRE(nativePtrB[4] == 4);

    // Create a third module from the second one
    wasm::WAVMWasmModule moduleC;
    moduleC.bindToFunctionNoZygote(m);

    // Restore from snapshot
    if (mode == "memory") {
        moduleC.restoreFromMemory(memoryData);
    } else if (mode == "file") {
        moduleC.restoreFromFile(filePath);
    } else {
        moduleC.restoreFromState(stateKey, stateSize);
    }

    // Check size of restored memory is as expected
    size_t actualSizeBytesC = moduleB.getMemorySizeBytes();
    REQUIRE(actualSizeBytesC == expectedSizeBytes);

    // Check writes to memory are visible in restored module
    uint8_t* nativePtrC = moduleC.wasmPointerToNative(wasmPtr);
    REQUIRE(nativePtrC[0] == 0);
    REQUIRE(nativePtrC[1] == 1);
    REQUIRE(nativePtrC[2] == 2);
    REQUIRE(nativePtrC[3] == 3);
    REQUIRE(nativePtrC[4] == 4);

    // Execute all of them
    bool successA = moduleA.execute(m);
    REQUIRE(successA);

    bool successB = moduleB.execute(m);
    REQUIRE(successB);

    bool successC = moduleC.execute(m);
    REQUIRE(successC);
}
}
