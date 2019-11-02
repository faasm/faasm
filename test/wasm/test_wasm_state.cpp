#include <catch/catch.hpp>
#include <wasm/WasmModule.h>
#include <util/func.h>
#include <emulator/emulator.h>
#include <redis/Redis.h>
#include <util/memory.h>

namespace tests {
    TEST_CASE("Test loading state into wasm module", "[wasm]") {
        wasm::WasmModule moduleA;
        wasm::WasmModule moduleB;

        const message::Message call = util::messageFactory("demo", "echo");
        moduleA.bindToFunction(call);
        moduleB.bindToFunction(call);

        // We have to make sure emulator is using the right user
        const std::string user = getEmulatorUser();
        const std::string key = "wasm_state_test";
        
        // Set up some state value
        long stateSize = 3 * util::HOST_PAGE_SIZE;
        std::vector<uint8_t> value(stateSize);
        std::fill(value.data(), value.data() + stateSize, 1);

        state::State &s = state::getGlobalState();
        auto kv = s.getKV(user, key, stateSize);
        
        // Write the value to redis
        redis::Redis &redis = redis::Redis::getState();
        redis.set(kv->key, value);

        // Map one region at the bottom of the state
        moduleA.mmapKey(kv, 10, 5);

        // Map anoter region crossing a page boundary
        moduleB.mmapKey(kv, 2 * util::HOST_PAGE_SIZE - 5, 123);
        
        // Get pointers slightly misaligned with these regions
        uint8_t *actualA = kv->getSegment(11, 3);
        uint8_t *actualB = kv->getSegment(2 * util::HOST_PAGE_SIZE - 2, 100);

        REQUIRE(actualA[0] == 1);
        REQUIRE(actualB[0] == 1);
    }
}
