#include <catch2/catch.hpp>

#include "fixtures.h"
#include "utils.h"

#include <boost/filesystem.hpp>

#include <faabric/proto/faabric.pb.h>
#include <faabric/runner/FaabricMain.h>
#include <faabric/snapshot/SnapshotClient.h>
#include <faabric/util/config.h>
#include <faabric/util/environment.h>
#include <faabric/util/func.h>
#include <faabric/util/memory.h>
#include <faabric/util/snapshot.h>
#include <faabric/util/testing.h>

#include <conf/FaasmConfig.h>
#include <faaslet/Faaslet.h>
#include <wavm/WAVMWasmModule.h>

using namespace wasm;

namespace tests {

class WasmSnapTestFixture
  : public RedisFixture
  , public SchedulerFixture
  , public SnapshotRegistryFixture
  , public ConfFixture
{
  public:
    WasmSnapTestFixture() { wasm::getWAVMModuleCache().clear(); }

    ~WasmSnapTestFixture() { wasm::getWAVMModuleCache().clear(); }
};

TEST_CASE_METHOD(WasmSnapTestFixture,
                 "Test snapshot and restore for wasm module",
                 "[wasm][snapshot]")
{
    std::string user = "demo";
    std::string function = "zygote_check";
    faabric::Message m = faabric::util::messageFactory(user, function);

    // Create the full module
    wasm::WAVMWasmModule moduleA;
    moduleA.bindToFunction(m);

    // Modify the memory of the module to check changes are propagated
    uint32_t memSize = WASM_BYTES_PER_PAGE + GUARD_REGION_SIZE;
    uint32_t wasmPtr = moduleA.growMemory(memSize);
    uint8_t* nativePtr = moduleA.wasmPointerToNative(wasmPtr);
    nativePtr[0] = 0;
    nativePtr[1] = 1;
    nativePtr[2] = 2;
    nativePtr[3] = 3;
    nativePtr[4] = 4;

    // Add a guard region to make sure these can be propagated
    uint32_t guardRegionOffset =
      moduleA.createMemoryGuardRegion(wasmPtr + WASM_BYTES_PER_PAGE);
    REQUIRE(guardRegionOffset ==
            wasmPtr + WASM_BYTES_PER_PAGE + GUARD_REGION_SIZE);

    size_t expectedSizeBytes = moduleA.getMemorySizeBytes();

    // Serialise to state
    std::string stateKey = moduleA.snapshot();

    // Create the module to be restored but don't execute zygote
    wasm::WAVMWasmModule moduleB;
    moduleB.bindToFunctionNoZygote(m);

    // Restore from snapshot
    moduleB.restore(stateKey);

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
    moduleC.restore(stateKey);

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
    int returnValueA = moduleA.executeFunction(m);
    REQUIRE(returnValueA == 0);

    int returnValueB = moduleB.executeFunction(m);
    REQUIRE(returnValueB == 0);

    int returnValueC = moduleC.executeFunction(m);
    REQUIRE(returnValueC == 0);
}

TEST_CASE_METHOD(WasmSnapTestFixture,
                 "Test reset from snapshot key",
                 "[wasm][snapshot]")
{
    std::string user = "demo";
    std::string function = "echo";
    faabric::Message m = faabric::util::messageFactory(user, function);

    // Set up some dummy data
    std::vector<uint8_t> dummyDataA(100, 1);
    std::vector<uint8_t> dummyDataB(100, 2);

    std::string snapshotKey = "foobar-snap";

    size_t snapSize = 64 * faabric::util::HOST_PAGE_SIZE;

    // Set up snapshot
    auto snap = std::make_shared<faabric::util::SnapshotData>(snapSize);

    // Write dummy data to the snapshot and make restorable
    snap->copyInData(dummyDataB);

    reg.registerSnapshot(snapshotKey, snap);

    SECTION("Wasm module")
    {
        wasm::WAVMWasmModule module;
        module.bindToFunction(m);

        size_t postBindSize = module.getMemorySizeBytes();
        uint8_t* memoryBase = module.getMemoryBase();

        // Record what the original data looks like
        std::vector<uint8_t> defaultData(memoryBase,
                                         memoryBase + dummyDataA.size());

        // Write some dummy data into memory (to check it gets overwritten)
        std::memcpy(memoryBase, dummyDataA.data(), dummyDataA.size());

        // Check the dummy data isn't the same as the default data
        REQUIRE(dummyDataA != defaultData);
        REQUIRE(dummyDataB != defaultData);

        SECTION("No snapshot key")
        {
            module.reset(m, "");

            // Check size is reset to default
            REQUIRE(module.getMemorySizeBytes() == postBindSize);

            // Check data overwritten with default
            std::vector<uint8_t> dataAfter(memoryBase,
                                           memoryBase + dummyDataA.size());
            REQUIRE(dataAfter == defaultData);
        }

        SECTION("With snapshot key")
        {
            // Reset the module and check it has the dummy data
            module.reset(m, snapshotKey);

            std::vector<uint8_t> dataAfter(memoryBase,
                                           memoryBase + dummyDataA.size());
            REQUIRE(dataAfter == dummyDataB);
        }
    }

    SECTION("Faaslet")
    {
        faaslet::Faaslet f(m);

        size_t defaultMemSize = f.module->getMemorySizeBytes();

        std::string resetKey = f.getLocalResetSnapshotKey();
        REQUIRE(!resetKey.empty());

        // Load the snapshot and check it's the right size by default
        auto initialSnap = reg.getSnapshot(resetKey);
        REQUIRE(initialSnap->getSize() == defaultMemSize);

        // Overwrite the snapshot to force the faaslet to restore ours
        reg.registerSnapshot(resetKey, snap);

        // Reset the faaslet, check dummy data is set in the module memory
        f.reset(m);

        uint8_t* memoryBase = f.module->getMemoryBase();
        std::vector<uint8_t> dataAfter(memoryBase,
                                       memoryBase + dummyDataB.size());

        REQUIRE(dataAfter == dummyDataB);
        REQUIRE(f.module->getCurrentBrk() == snapSize);

        f.shutdown();
    }
}
}
