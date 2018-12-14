#include <catch/catch.hpp>
#include "utils.h"
#include <infra/infra.h>

using namespace infra;

namespace tests {

    TEST_CASE("Test simple state get/set", "[state]") {
        State s;
        std::string key = "test_state_new";

        StateKeyValue *kv = s.getKV(key);

        // Get
        std::vector<uint8_t> actual = kv->get();
        REQUIRE(actual.empty());

        // Update
        std::vector<uint8_t> values = {0, 1, 2, 3, 4};
        kv->set(values);

        // Check that getting returns the update
        REQUIRE(kv->get() == values);

        // Check that the underlying key in Redis isn't changed
        REQUIRE(redisState.get(key).empty());

        // Check that when pushed, the update is pushed to redis
        kv->push();
        REQUIRE(kv->get() == values);
        REQUIRE(redisState.get(key) == values);
    }

    TEST_CASE("Test get/ set segment", "[state]") {
        State s;
        std::string key = "test_state_segment";
        StateKeyValue *kv = s.getKV(key);

        // Set up and push
        std::vector<uint8_t> values = {0, 0, 1, 1, 2, 2, 3, 3, 4, 4};
        kv->set(values);
        kv->push();

        // Get and check
        REQUIRE(kv->get() == values);

        // Update a subsection
        std::vector<uint8_t> update = {8, 8, 8};
        kv->setSegment(3, update);

        // Check changed locally but not in redis
        std::vector<uint8_t> expected = {0, 0, 1, 8, 8, 8, 3, 3, 4, 4};
        REQUIRE(redisState.get(key) == values);
        REQUIRE(kv->get() == expected);
        REQUIRE(kv->getSegment(3, 3) == update);

        // Run push and check redis updated
        kv->push();
        REQUIRE(redisState.get(key) == expected);
    }

    TEST_CASE("Test set segment interactions", "[state]") {
        State s;
        std::string key = "test_state_segment_interaction";
        StateKeyValue *kv = s.getKV(key);

        // Set up and push
        std::vector<uint8_t> values = {0, 1, 2, 3, 4, 5};
        kv->set(values);
        kv->push();

        // Get and check
        REQUIRE(kv->get() == values);
        REQUIRE(redisState.get(key) == values);

        // Update a subsection
        std::vector<uint8_t> updateA = {7, 7};
        kv->setSegment(0, updateA);

        // Update a section directly in redis
        std::vector<uint8_t> updateB = {8, 8, 8};
        redisState.setRange(key, 3, updateB);

        // Check updates made individually
        std::vector<uint8_t> expectedA = {7, 7, 2, 3, 4, 5};
        std::vector<uint8_t> expectedB = {0, 1, 2, 8, 8, 8};

        // Check expectations
        REQUIRE(kv->get() == expectedA);
        REQUIRE(redisState.get(key) == expectedB);

        // Now push and check two updates have both been accepted
        kv->push();
        std::vector<uint8_t> expectedMerged = {7, 7, 2, 8, 8, 8};
        REQUIRE(redisState.get(key) == expectedMerged);
        REQUIRE(kv->get() == expectedMerged);
    }

    TEST_CASE("Test pushing all state", "[state]") {
        State s;
        std::string keyA = "test_multi_push_a";
        std::string keyB = "test_multi_push_b";

        StateKeyValue *kvA = s.getKV(keyA);
        StateKeyValue *kvB = s.getKV(keyB);

        // Set up and push
        std::vector<uint8_t> valuesA = {0, 1, 2, 3};
        std::vector<uint8_t> valuesB = {4, 5, 6};

        kvA->set(valuesA);
        kvB->set(valuesB);

        // Check neither in redis
        REQUIRE(redisState.get(keyA).empty());
        REQUIRE(redisState.get(keyB).empty());

        // Push all
        s.pushAll();

        // Check both now in redis
        REQUIRE(redisState.get(keyA) == valuesA);
        REQUIRE(redisState.get(keyB) == valuesB);
    }
}