#include <catch/catch.hpp>
#include "utils.h"
#include <wasm/wasm.h>

using namespace wasm;

namespace tests {

    TEST_CASE("Test simple state get/set", "[state]") {
        State s;
        std::string user = "test";
        std::string key = "state_new";

        // Fake time
        util::Clock &clock = util::getGlobalClock();
        clock.setFakeNow(clock.now());

        StateKeyValue *kv = s.getKV(user, key, 5);

        std::string actualKey = "test_state_new";

        // Get (should do nothing)
        std::vector<uint8_t> actual(5);
        kv->get(actual.data());

        // Update
        std::vector<uint8_t> values = {0, 1, 2, 3, 4};
        kv->set(values.data());

        // Check that getting returns the update
        kv->get(actual.data());
        REQUIRE(actual == values);

        // Check that the underlying key in Redis isn't changed
        REQUIRE(redisState.get(actualKey).empty());

        // Check that when pushed, the update is pushed to redis
        kv->pushFull();
        kv->get(actual.data());
        REQUIRE(actual == values);
        REQUIRE(redisState.get(actualKey) == values);
    }

    TEST_CASE("Test get/ set segment", "[state]") {
        State s;
        std::string user = "test";
        std::string key = "state_segment";
        StateKeyValue *kv = s.getKV(user, key, 10);

        std::string actualKey = "test_state_segment";

        // Set up and push
        std::vector<uint8_t> values = {0, 0, 1, 1, 2, 2, 3, 3, 4, 4};
        kv->set(values.data());
        kv->pushFull();

        // Get and check
        std::vector<uint8_t> actual(10);
        kv->get(actual.data());
        REQUIRE(actual == values);

        // Update a subsection
        std::vector<uint8_t> update = {8, 8, 8};
        kv->setSegment(3, update.data(), 3);

        // Check changed locally but not in redis
        std::vector<uint8_t> expected = {0, 0, 1, 8, 8, 8, 3, 3, 4, 4};
        kv->get(actual.data());
        REQUIRE(redisState.get(actualKey) == values);
        REQUIRE(actual == expected);

        std::vector<uint8_t> actualSegment(3);
        kv->getSegment(3, actualSegment.data(), 3);
        REQUIRE(actualSegment == update);

        // Run push and check redis updated
        kv->pushPartial();
        REQUIRE(redisState.get(actualKey) == expected);
    }

    TEST_CASE("Test set segment cannot be out of bounds", "[state]") {
        State s;
        std::string user = "test";
        std::string key = "state_extension";
        StateKeyValue *kv = s.getKV(user, key, 2);

        std::string actualKey = "test_state_oob";

        // Set a segment offset
        std::vector<uint8_t> update = {8, 8, 8};

        REQUIRE_THROWS(kv->setSegment(5, update.data(), 3));
    }

    TEST_CASE("Test pushing all state", "[state]") {
        State s;
        std::string userA = "test_a";
        std::string userB = "test_b";
        std::string keyA = "multi_push_a";
        std::string keyB = "multi_push_b";

        StateKeyValue *kvA = s.getKV(userA, keyA, 4);
        StateKeyValue *kvB = s.getKV(userB, keyB, 3);

        std::string actualKeyA = userA + "_" + keyA;
        std::string actualKeyB = userB + "_" + keyB;

        // Set up and push
        std::vector<uint8_t> valuesA = {0, 1, 2, 3};
        std::vector<uint8_t> valuesB = {4, 5, 6};

        kvA->set(valuesA.data());
        kvB->set(valuesB.data());

        // Check neither in redis
        REQUIRE(redisState.get(keyA).empty());
        REQUIRE(redisState.get(keyB).empty());

        // Push all
        s.pushAll();

        // Check both now in redis
        REQUIRE(redisState.get(actualKeyA) == valuesA);
        REQUIRE(redisState.get(actualKeyB) == valuesB);
    }

    TEST_CASE("Test getting/ pushing only happens when stale/ dirty", "[state]") {
        State s;
        std::string user = "test";
        std::string key = "dirty_push";
        StateKeyValue *kv = s.getKV(user, key, 4);
        std::vector<uint8_t> values = {0, 1, 2, 3};

        std::string actualKey = user + "_" + key;

        // Fix time
        util::Clock &c = util::getGlobalClock();
        const util::TimePoint now = c.now();
        c.setFakeNow(now);

        // Push and make sure reflected in redis
        kv->set(values.data());
        kv->pushFull();
        REQUIRE(redisState.get(actualKey) == values);

        // Now update in Redis directly
        std::vector<uint8_t> newValues = {5, 5, 5, 5};
        redisState.set(actualKey, newValues);

        // Get and check that remote value isn't pulled because it's not stale
        std::vector<uint8_t> actual(4);
        kv->get(actual.data());
        REQUIRE(actual == values);

        // Check that push also doesn't push to remote as not dirty
        kv->pushFull();
        REQUIRE(redisState.get(actualKey) == newValues);

        // Now advance time and make sure new value is retrieved
        c.setFakeNow(now + std::chrono::seconds(120));
        kv->get(actual.data());
        REQUIRE(actual == newValues);
    }

    TEST_CASE("Test stale values cleared", "[state]") {
        State s;
        std::string user = "stale";
        std::string key = "check";
        util::Clock &c = util::getGlobalClock();

        // Fix time
        const util::TimePoint now = c.now();
        c.setFakeNow(now);

        std::vector<uint8_t> value = {0, 1, 2, 3};
        StateKeyValue *kv = s.getKV(user, key, 4);

        // Push value to redis
        kv->set(value.data());
        kv->pushFull();

        // Try clearing, check nothing happens
        kv->clear();
        std::vector<uint8_t> actual(4);
        kv->get(actual.data());
        REQUIRE(actual == value);

        // Advance time a bit, check it doesn't get cleared
        std::chrono::seconds secsSmall(1);
        util::TimePoint newNow = now + secsSmall;
        c.setFakeNow(newNow);

        kv->clear();
        REQUIRE(!kv->isNew());

        // Advance time a lot and check it does get cleared
        std::chrono::seconds secsBig(120);
        newNow = now + secsBig;
        c.setFakeNow(newNow);

        kv->clear();
        REQUIRE(kv->isNew());
    }

    TEST_CASE("Test pulling only happens if stale") {
        State s;
        std::string user = "stale";
        std::string key = "pull_check";
        util::Clock &c = util::getGlobalClock();

        // Fix time
        const util::TimePoint now = c.now();
        c.setFakeNow(now);

        // Set up a value in redis
        std::vector<uint8_t> value = {0, 1, 2, 3, 4, 5};
        StateKeyValue *kv = s.getKV(user, key, 6);
        kv->set(value.data());
        kv->pushFull();

        // Now change the value in Redis
        std::vector<uint8_t> value2 = {2, 4, 2, 4, 2, 4};
        std::string actualKey = user + "_" + key;
        redisState.set(actualKey, value2);

        // Pull, check hasn't changed
        kv->pull();
        std::vector<uint8_t> actual(6);
        kv->get(actual.data());
        REQUIRE(actual == value);

        // Move time forward then pull again and check updated
        c.setFakeNow(now + std::chrono::seconds(30));
        kv->pull();
        kv->get(actual.data());
        REQUIRE(actual == value2);
    }

    void checkActionResetsIdleness(std::string actionType) {
        redisState.flushAll();

        State s;
        std::string user = "idle";
        std::string key = "check";
        std::string actualKey = user + "_" + key;

        util::Clock &c = util::getGlobalClock();
        const util::TimePoint now = c.now();
        c.setFakeNow(now);

        // Check not idle by default (i.e. won't get cleared)
        StateKeyValue *const kv = s.getKV(user, key, 3);
        std::vector<uint8_t> values = {1, 2, 3};
        kv->set(values.data());
        kv->pushFull();
        kv->clear();

        REQUIRE(!kv->isNew());

        // Move time on, but then perform some action
        c.setFakeNow(now + std::chrono::seconds(180));

        std::vector<uint8_t> actual;
        if (actionType == "get") {
            actual.reserve(3);
            kv->get(actual.data());
        } else if (actionType == "getSegment") {
            actual.reserve(2);
            kv->getSegment(1, actual.data(), 2);
        } else if (actionType == "set") {
            actual = {4, 5, 6};
            kv->set(actual.data());
        } else if (actionType == "setSegment") {
            actual = {4, 4};
            kv->setSegment(1, actual.data(), 2);
        } else {
            throw std::runtime_error("Unrecognised action type");
        }

        // Try clearing, check not cleared
        kv->clear();

        // Check not counted as stale
        REQUIRE(!kv->isNew());

        // Move time on again without any action, check is stale and gets cleared
        c.setFakeNow(now + std::chrono::seconds(360));
        kv->clear();
        REQUIRE(kv->isNew());

    }

    TEST_CASE("Check idleness reset with get", "[state]") {
        checkActionResetsIdleness("get");
    }

    TEST_CASE("Check idleness reset with getSegment", "[state]") {
        checkActionResetsIdleness("getSegment");
    }

    TEST_CASE("Check idleness reset with set", "[state]") {
        checkActionResetsIdleness("set");
    }

    TEST_CASE("Check idleness reset with setSegment", "[state]") {
        checkActionResetsIdleness("setSegment");
    }

    TEST_CASE("Test merging segments", "[state]") {
        SegmentSet setIn;
        SegmentSet expected;

        // Segments next to each other (inserted out of order)
        setIn.insert(Segment(5, 10));
        setIn.insert(Segment(0, 5));
        expected.insert(Segment(0, 10));

        // Segments that overlap  (inserted out of order)
        setIn.insert(Segment(15, 18));
        setIn.insert(Segment(14, 16));
        setIn.insert(Segment(19, 25));
        setIn.insert(Segment(15, 20));
        expected.insert(Segment(14, 25));

        // Segments that don't overlap
        setIn.insert(Segment(30, 40));
        setIn.insert(Segment(41, 50));
        setIn.insert(Segment(70, 90));
        expected.insert(Segment(30, 40));
        expected.insert(Segment(41, 50));
        expected.insert(Segment(70, 90));

        SegmentSet actual = StateKeyValue::mergeSegments(setIn);
        REQUIRE(actual == expected);
    }

    TEST_CASE("Test merging segments does nothing with sets with one element", "[state]") {
        SegmentSet setIn = {Segment(10, 15)};
        SegmentSet expected = setIn;
        REQUIRE(StateKeyValue::mergeSegments(setIn) == expected);
    }

    TEST_CASE("Test merging segments manages sets with two elements", "[state]") {
        SegmentSet setIn = {Segment(10, 15), Segment(11, 18)};
        SegmentSet expected = {Segment(10, 18)};
        REQUIRE(StateKeyValue::mergeSegments(setIn) == expected);
    }

    TEST_CASE("Test merging segments does nothing with empty set", "[state]") {
        SegmentSet setIn;
        SegmentSet expected = setIn;
        REQUIRE(StateKeyValue::mergeSegments(setIn) == expected);
    }
}