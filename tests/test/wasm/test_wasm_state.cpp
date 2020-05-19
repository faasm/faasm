#include <catch/catch.hpp>

#include "utils.h"

#include <wavm/WAVMWasmModule.h>
#include <util/func.h>
#include <emulator/emulator.h>
#include <redis/Redis.h>
#include <util/memory.h>
#include <util/state.h>

namespace tests {
    void _checkMapping(
            wasm::WAVMWasmModule &module,
            std::shared_ptr<state::StateKeyValue> &kv,
            long offset,
            long size,
            std::vector<uint8_t> &expected
    ) {
        // Check mapped whole region

        U32 wasmPtr = module.mmapKey(kv, offset, size);
        U8 *hostPtr = Runtime::memoryArrayPtr<U8>(module.defaultMemory, (Uptr) wasmPtr, (Uptr) size);

        std::vector<uint8_t> actual(hostPtr, hostPtr + size);
        REQUIRE(actual == expected);
    }

    TEST_CASE("Test loading state into wasm module", "[wasm]") {
        cleanSystem();
        
        wasm::WAVMWasmModule moduleA;
        wasm::WAVMWasmModule moduleB;

        const message::Message call = util::messageFactory("demo", "echo");
        moduleA.bindToFunction(call);
        moduleB.bindToFunction(call);

        // Set up some state value
        long stateSize = 5 * util::HOST_PAGE_SIZE;
        std::vector<uint8_t> value(stateSize);
        std::fill(value.data(), value.data() + stateSize, 1);

        // Write significant values at known points
        long offsetA1 = 10;
        long offsetA2 = util::HOST_PAGE_SIZE + 2;
        long offsetB1 = 2 * util::HOST_PAGE_SIZE - 5;
        long offsetB2 = 3 * util::HOST_PAGE_SIZE - 5;

        uint8_t markerA1 = 6;
        uint8_t markerA2 = 7;
        uint8_t markerB1 = 8;
        uint8_t markerB2 = 9;

        value[offsetA1] = markerA1;
        value[offsetA2] = markerA2;
        value[offsetB1] = markerB1;
        value[offsetB2] = markerB2;

        // Get the kv
        state::State &s = state::getGlobalState();
        const std::string user = getEmulatorUser();
        const std::string key = "wasm_state_test";
        auto kv = s.getKV(user, key, stateSize);

        // Write the value to the key-value
        kv->set(value.data());

        // Check mapping the whole region
        _checkMapping(moduleA, kv, 0, stateSize, value);

        // Map a segment of that module to the same value
        std::vector<uint8_t> expectedA2 = {1, markerA1, 1, 1};
        _checkMapping(moduleA, kv, offsetA1 - 1, 4, expectedA2);

        // Map a segment of a different module and check
        std::vector<uint8_t> expectedB1 = {1, 1, 1, markerB1, 1, 1, 1};
        _checkMapping(moduleB, kv, offsetB1 - 3, 7, expectedB1);

        // Map a segment of a different module and check
        std::vector<uint8_t> expectedB2 = {1, 1, 1, 1, 1, markerB2, 1};
        _checkMapping(moduleB, kv, offsetB2 - 5, 7, expectedB2);
    }
    
    TEST_CASE("Test mapping state chunk from remote state", "[state]") {
        cleanSystem();

        // Set up a non-page-aligned size
        long stateSize = (util::HOST_PAGE_SIZE * 2) + 23;
        long expectedAllocatedSize = util::HOST_PAGE_SIZE * 3;
        std::vector<uint8_t> values(stateSize, 3);

        // Set up a chunk at the end
        size_t chunkOffset = stateSize - 10;
        size_t chunkSize = 5;
        values[chunkOffset] = 4;
        values[chunkOffset + 1] = 5;
        std::vector<uint8_t> expected = {4, 5, 3, 3, 3};

        std::string user = getEmulatorUser();
        std::string key = "chunk_map";

        // Set up a remote state server
        DummyStateServer server;
        server.dummyUser = user;
        server.dummyKey = "chunk_map";
        server.dummyData = values;

        // Expect one pull
        server.start(1);

        // Check allocated memory is aligned up to a page boundary
        const std::shared_ptr<state::StateKeyValue> &remoteKv = server.getRemoteKv();
        size_t memSize = remoteKv->getSharedMemorySize();
        REQUIRE(memSize == expectedAllocatedSize);

        // Create a wasm module
        wasm::WAVMWasmModule module;
        const message::Message call = util::messageFactory("demo", "echo");
        module.bindToFunction(call);

        // Map the chunk locally
        const std::shared_ptr<state::StateKeyValue> &localKv = server.getLocalKv();
        uint32_t wasmPtr = module.mmapKey(localKv, chunkOffset, chunkSize);

        // Check wasm memory
        U8 *hostPtr = Runtime::memoryArrayPtr<U8>(module.defaultMemory, (Uptr) wasmPtr, (Uptr) chunkSize);
        std::vector<uint8_t> actual(hostPtr, hostPtr + chunkSize);
        REQUIRE(actual == expected);

        // Allow the server to shut down
        server.wait();
    }
}
