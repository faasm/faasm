#include <catch2/catch.hpp>

#include "fixtures.h"
#include "utils.h"

#include <faabric/util/func.h>
#include <faabric/util/memory.h>
#include <faabric/util/state.h>
#include <wavm/WAVMWasmModule.h>

using namespace WAVM;

namespace tests {

class WasmStateTestFixture : public StateFixture
{
  public:
    void checkMapping(wasm::WAVMWasmModule& module,
                      std::shared_ptr<faabric::state::StateKeyValue>& kv,
                      long offset,
                      long size,
                      std::vector<uint8_t>& expected)
    {
        // Check mapped whole region
        U32 wasmPtr = module.mapSharedStateMemory(kv, offset, size);
        U8* hostPtr = Runtime::memoryArrayPtr<U8>(
          module.defaultMemory, (Uptr)wasmPtr, (Uptr)size);

        std::vector<uint8_t> actual(hostPtr, hostPtr + size);
        REQUIRE(actual == expected);
    }
};

TEST_CASE_METHOD(WasmStateTestFixture,
                 "Test loading state into wasm module",
                 "[wasm]")
{
    wasm::WAVMWasmModule moduleA;
    wasm::WAVMWasmModule moduleB;

    faabric::Message call = faabric::util::messageFactory("demo", "echo");
    moduleA.bindToFunction(call);
    moduleB.bindToFunction(call);

    // Set up some state value
    long stateSize = 5 * faabric::util::HOST_PAGE_SIZE;
    std::vector<uint8_t> value(stateSize);
    std::fill(value.data(), value.data() + stateSize, 1);

    // Write significant values at known points
    long offsetA1 = 10;
    long offsetA2 = faabric::util::HOST_PAGE_SIZE + 2;
    long offsetB1 = 2 * faabric::util::HOST_PAGE_SIZE - 5;
    long offsetB2 = 3 * faabric::util::HOST_PAGE_SIZE - 5;

    uint8_t markerA1 = 6;
    uint8_t markerA2 = 7;
    uint8_t markerB1 = 8;
    uint8_t markerB2 = 9;

    value[offsetA1] = markerA1;
    value[offsetA2] = markerA2;
    value[offsetB1] = markerB1;
    value[offsetB2] = markerB2;

    // Get the kv
    const std::string user = "demo";
    const std::string key = "wasm_state_test";
    auto kv = state.getKV(user, key, stateSize);

    // Write the value to the key-value
    kv->set(value.data());

    // Check mapping the whole region
    checkMapping(moduleA, kv, 0, stateSize, value);

    // Map a segment of that module to the same value
    std::vector<uint8_t> expectedA2 = { 1, markerA1, 1, 1 };
    checkMapping(moduleA, kv, offsetA1 - 1, 4, expectedA2);

    // Map a segment of a different module and check
    std::vector<uint8_t> expectedB1 = { 1, 1, 1, markerB1, 1, 1, 1 };
    checkMapping(moduleB, kv, offsetB1 - 3, 7, expectedB1);

    // Map a segment of a different module and check
    std::vector<uint8_t> expectedB2 = { 1, 1, 1, 1, 1, markerB2, 1 };
    checkMapping(moduleB, kv, offsetB2 - 5, 7, expectedB2);
}
}
