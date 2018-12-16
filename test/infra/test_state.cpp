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
    
    TEST_CASE("Test stale values cleared", "[state]") {
        State s;
        std::string key = "stale_check";
        util::Clock &c = util::getGlobalClock();

        // Fix time
        const util::TimePoint now = c.now();
        c.setFakeNow(now);

        std::vector<uint8_t> value = {0, 1, 2, 3};
        StateKeyValue * kv = s.getKV(key);

        // Push value to redis
        kv->set(value);
        kv->push();

        // Try clearing, check nothing happens
        kv->clear();
        REQUIRE(kv->get() == value);
        REQUIRE(kv->getLocalValueSize() == value.size());

        // Advance time a bit, check it doesn't get cleared
        std::chrono::seconds secsSmall(1);
        util::TimePoint newNow = now + secsSmall;
        c.setFakeNow(newNow);

        kv->clear();
        REQUIRE(kv->getLocalValueSize() == value.size());

        // Advance time a lot and check it does get cleared
        std::chrono::seconds secsBig(120);
        newNow = now + secsBig;
        c.setFakeNow(newNow);

        kv->clear();
        REQUIRE(kv->getLocalValueSize() == 0);
    }

    TEST_CASE("Test pulling only happens if stale") {
        State s;
        std::string key = "stale_pull_check";
        util::Clock &c = util::getGlobalClock();

        // Fix time
        const util::TimePoint now = c.now();
        c.setFakeNow(now);

        // Set up a value in redis
        std::vector<uint8_t> value = {0, 1, 2, 3, 4, 5};
        StateKeyValue * kv = s.getKV(key);
        kv->set(value);
        kv->push();

        // Now change the value in Redis
        std::vector<uint8_t> value2 = {2, 4};
        redisState.set(key, value2);

        // Pull, check hasn't changed
        kv->pull();
        REQUIRE(kv->get() == value);

        // Move time forward then pull again and check updated
        c.setFakeNow(now + std::chrono::seconds(30));
        kv->pull();
        REQUIRE(kv->get() == value2);
    }
}