#include <catch/catch.hpp>

#include <infra/infra.h>

using namespace infra;

namespace tests {

    TEST_CASE("Test simple state get/set", "[state]") {
        State s;
        std::string newKey = "test_state_new";

        StateKeyValue *kv = s.getKV(newKey);

        REQUIRE(!kv->isDirty());

        // Get the key value via state
        std::vector<uint8_t> actual = kv->get();
        REQUIRE(actual.empty());

        // Update via state
        std::vector<uint8_t> values = {0, 1, 2, 3, 4};
        kv->set(values);
        REQUIRE(kv->isDirty());

        // Check that getting returns the update
        REQUIRE(kv->get() == values);

        // Check that the underlying key in Redis isn't changed
        Redis *redis = Redis::getThreadState();
        REQUIRE(redis->get(newKey).empty());

        // Check that when synchronised, the update is pushed to redis
        kv->sync();
        REQUIRE(!kv->isDirty());
        REQUIRE(kv->get() == values);
        REQUIRE(redis->get(newKey) == values);
    }

    TEST_CASE("Test get/ set segment", "[state]") {

    }
}