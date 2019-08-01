#include <catch/catch.hpp>

#include "utils.h"

#include <redis/Redis.h>
#include <util/memory.h>
#include <util/environment.h>
#include <util/config.h>
#include <state/State.h>
#include <sys/mman.h>
#include <util/state.h>
#include <emulator/emulator.h>

using namespace state;

namespace tests {
    util::TimePoint timeNow;
    static int staticCount = 0;

    std::shared_ptr<StateKeyValue> setupKV(size_t size) {
        cleanSystem();

        // We have to make sure emulator is using the right user
        const std::string user = getEmulatorUser();

        staticCount++;

        const std::string stateKey = "state_key_" + std::to_string(staticCount);

        // Get state and remove key if already exists
        State &s = getGlobalState();
        auto kv = s.getKV(user, stateKey, size);

        // Fix time
        util::Clock &c = util::getGlobalClock();
        timeNow = c.now();
        c.setFakeNow(timeNow);

        return kv;
    }

    TEST_CASE("Test simple state get/set", "[state]") {
        redis::Redis &redisState = redis::Redis::getState();
        auto kv = setupKV(5);

        // Get (should do nothing)
        std::vector<uint8_t> actual(5);
        std::vector<uint8_t> expected = {0, 0, 0, 0, 0};

        kv->pull(true);
        kv->get(actual.data());
        REQUIRE(actual == expected);

        // Update
        std::vector<uint8_t> values = {0, 1, 2, 3, 4};
        kv->set(values.data());

        // Check that getting returns the update
        kv->get(actual.data());
        REQUIRE(actual == values);

        // Check that the underlying key in Redis isn't changed
        REQUIRE(redisState.get(kv->key).empty());

        // Check that when pushed, the update is pushed to redis
        kv->pushFull();
        kv->get(actual.data());
        REQUIRE(actual == values);
        REQUIRE(redisState.get(kv->key) == values);
    }

    TEST_CASE("Test get/ set segment", "[state]") {
        redis::Redis &redisState = redis::Redis::getState();
        auto kv = setupKV(10);

        // Set up and push
        std::vector<uint8_t> values = {0, 0, 1, 1, 2, 2, 3, 3, 4, 4};
        kv->set(values.data());
        kv->pushFull();

        // Get and check
        std::vector<uint8_t> actual(10);
        kv->get(actual.data());
        REQUIRE(actual == values);
        REQUIRE(redisState.get(kv->key) == values);

        // Update a subsection
        std::vector<uint8_t> update = {8, 8, 8};
        kv->setSegment(3, update.data(), 3);

        // Check changed locally but not in redis
        std::vector<uint8_t> expected = {0, 0, 1, 8, 8, 8, 3, 3, 4, 4};
        kv->get(actual.data());
        REQUIRE(actual == expected);
        REQUIRE(redisState.get(kv->key) == values);

        // Try getting a segment
        std::vector<uint8_t> actualSegment(3);
        kv->getSegment(3, actualSegment.data(), 3);
        REQUIRE(actualSegment == update);

        // Run push and check redis updated
        kv->pushPartial();
        REQUIRE(redisState.get(kv->key) == expected);
    }

    TEST_CASE("Test marking segments dirty", "[state]") {
        redis::Redis &redisState = redis::Redis::getState();
        auto kv = setupKV(10);

        // Set up and push
        std::vector<uint8_t> values = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        kv->set(values.data());
        kv->pushFull();

        // Get pointer, update
        uint8_t *ptr = kv->get();
        ptr[0] = 8;
        ptr[5] = 7;
        kv->pushPartial();

        // Check nothing happens
        REQUIRE(redisState.get(kv->key) == values);

        // Mark region as dirty, check update happens
        kv->flagSegmentDirty(0, 2);
        kv->pushPartial();
        values.at(0) = 8;
        REQUIRE(redisState.get(kv->key) == values);

        // Mark other region and check also updated
        kv->flagSegmentDirty(5, 6);
        kv->pushPartial();
        values.at(5) = 7;
        REQUIRE(redisState.get(kv->key) == values);
    }


    TEST_CASE("Test set segment cannot be out of bounds", "[state]") {
        auto kv = setupKV(2);

        // Set a segment offset
        std::vector<uint8_t> update = {8, 8, 8};

        REQUIRE_THROWS(kv->setSegment(5, update.data(), 3));
    }

    TEST_CASE("Test partially setting just first/ last element", "[state]") {
        redis::Redis &redisState = redis::Redis::getState();
        auto kv = setupKV(5);

        // Set up and push
        std::vector<uint8_t> values = {0, 1, 2, 3, 4};
        kv->set(values.data());
        kv->pushFull();

        // Update just the last
        std::vector<uint8_t> update = {8};
        kv->setSegment(4, update.data(), 1);

        kv->pushPartial();
        std::vector<uint8_t> expected = {0, 1, 2, 3, 8};
        REQUIRE(redisState.get(kv->key) == expected);

        // Update the first
        kv->setSegment(0, update.data(), 1);

        kv->pushPartial();
        expected = {8, 1, 2, 3, 8};
        REQUIRE(redisState.get(kv->key) == expected);

        // Update both
        update = {6};
        kv->setSegment(0, update.data(), 1);
        kv->setSegment(4, update.data(), 1);

        kv->pushPartial();
        expected = {6, 1, 2, 3, 6};
        REQUIRE(redisState.get(kv->key) == expected);
    }

    TEST_CASE("Test pushing all state", "[state]") {
        redis::Redis &redisState = redis::Redis::getState();

        State s;
        std::string userA = "test_a";
        std::string userB = "test_b";
        std::string keyA = "multi_push_a";
        std::string keyB = "multi_push_b";

        auto kvA = s.getKV(userA, keyA, 4);
        auto kvB = s.getKV(userB, keyB, 3);

        std::string actualStateKeyA = util::keyForUser(userA, keyA);
        std::string actualStateKeyB = util::keyForUser(userB, keyB);

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
        REQUIRE(redisState.get(actualStateKeyA) == valuesA);
        REQUIRE(redisState.get(actualStateKeyB) == valuesB);
    }

    void checkPulling(bool async) {
        auto kv = setupKV(4);
        std::vector<uint8_t> values = {0, 1, 2, 3};
        util::SystemConfig &conf = util::getSystemConfig();

        // Push and make sure reflected in redis
        kv->set(values.data());
        kv->pushFull();

        redis::Redis &redisState = redis::Redis::getState();
        REQUIRE(redisState.get(kv->key) == values);

        // Now update in Redis directly
        std::vector<uint8_t> newValues = {5, 5, 5, 5};
        redisState.set(kv->key, newValues);

        // Get and check whether the remote is pulled
        std::vector<uint8_t> actual(4);
        kv->pull(async);
        kv->get(actual.data());

        if (async) {
            REQUIRE(actual == values);
        } else {
            REQUIRE(actual == newValues);
        }

        // Now advance time and make sure new value is retrieved for async
        util::Clock &c = util::getGlobalClock();
        long bigStep = conf.stateStaleThreshold + 100;
        if (async) {
            c.setFakeNow(timeNow + std::chrono::milliseconds(bigStep));
            kv->pull(true);
            kv->get(actual.data());
            REQUIRE(actual == newValues);
        }
    }

    TEST_CASE("Test async pulling only happens when stale", "[state]") {
        checkPulling(true);
    }

    TEST_CASE("Test sync pulling always happens", "[state]") {
        checkPulling(false);
    }

    TEST_CASE("Test pushing only happens when dirty", "[state]") {
        redis::Redis &redisState = redis::Redis::getState();

        auto kv = setupKV(4);

        std::vector<uint8_t> values = {0, 1, 2, 3};
        kv->set(values.data());
        kv->pushFull();

        // Change in redis directly
        std::vector<uint8_t> newValues = {3, 4, 5, 6};
        redisState.set(kv->key, newValues);

        // Push and make sure redis not changed as it's not dirty
        kv->pushFull();
        REQUIRE(redisState.get(kv->key) == newValues);

        // Now change locally and check push happens
        std::vector<uint8_t> newValues2 = {7, 7, 7, 7};
        kv->set(newValues2.data());
        kv->pushFull();
        REQUIRE(redisState.get(kv->key) == newValues2);
    }

    TEST_CASE("Test stale values cleared", "[state]") {
        auto kv = setupKV(4);

        util::Clock &c = util::getGlobalClock();

        std::vector<uint8_t> value = {0, 1, 2, 3};

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
        util::TimePoint newNow = timeNow + secsSmall;
        c.setFakeNow(newNow);

        kv->clear();
        REQUIRE(!kv->empty());

        // Advance time a lot and check it does get cleared
        std::chrono::seconds secsBig(1000);
        newNow = timeNow + secsBig;
        c.setFakeNow(newNow);

        kv->clear();
        REQUIRE(kv->empty());
    }

    void checkActionResetsIdleness(std::string actionType) {
        auto kv = setupKV(3);

        util::Clock &c = util::getGlobalClock();

        // Check not idle by default (i.e. won't get cleared)
        std::vector<uint8_t> values = {1, 2, 3};
        kv->set(values.data());
        kv->pushFull();
        kv->clear();

        REQUIRE(!kv->empty());

        // Move time on, but then perform some action
        c.setFakeNow(timeNow + std::chrono::seconds(180));

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
        REQUIRE(!kv->empty());

        // Move time on again without any action, check is stale and gets cleared
        c.setFakeNow(timeNow + std::chrono::seconds(1000));
        kv->clear();
        REQUIRE(kv->empty());

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

    TEST_CASE("Test mapping shared memory", "[state]") {
        // Set up the KV
        auto kv = setupKV(5);
        std::vector<uint8_t> value = {0, 1, 2, 3, 4};
        kv->set(value.data());

        // Get a pointer to the shared memory and update
        uint8_t *sharedRegion = kv->get();
        sharedRegion[0] = 5;
        sharedRegion[2] = 5;

        // Map some shared memory areas
        int memSize = 2 * util::HOST_PAGE_SIZE;
        void *mappedRegionA = mmap(nullptr, memSize, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        auto byteRegionA = static_cast<uint8_t *>(mappedRegionA);
        kv->mapSharedMemory(mappedRegionA);
        void *mappedRegionB = mmap(nullptr, memSize, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        kv->mapSharedMemory(mappedRegionB);
        auto byteRegionB = static_cast<uint8_t *>(mappedRegionB);

        // Check shared memory regions reflect state

        std::vector<uint8_t> expected = {5, 1, 5, 3, 4};
        for (int i = 0; i < 5; i++) {
            REQUIRE(byteRegionA[i] == expected.at(i));
            REQUIRE(byteRegionB[i] == expected.at(i));
        }

        // Now update the pointer directly from both
        byteRegionA[2] = 6;
        byteRegionB[3] = 7;
        sharedRegion[4] = 8;
        std::vector<uint8_t> expected2 = {5, 1, 6, 7, 8};
        for (int i = 0; i < 5; i++) {
            REQUIRE(sharedRegion[i] == expected2.at(i));
            REQUIRE(byteRegionA[i] == expected2.at(i));
            REQUIRE(byteRegionB[i] == expected2.at(i));
        }

        // Now unmap the memory of one shared region
        kv->unmapSharedMemory(mappedRegionA);

        // Check original and shared regions still intact
        for (int i = 0; i < 5; i++) {
            REQUIRE(sharedRegion[i] == expected2.at(i));
            REQUIRE(byteRegionB[i] == expected2.at(i));
        }
    }

    TEST_CASE("Test mapping shared memory offsets", "[state]") {
        // Set up the KV
        auto kv = setupKV(7);
        std::vector<uint8_t> value = {0, 1, 2, 3, 4, 5, 6};
        kv->set(value.data());

        // Map a shared region
        void *mappedRegionA = mmap(nullptr, util::HOST_PAGE_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        void *mappedRegionB = mmap(nullptr, util::HOST_PAGE_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        kv->mapSharedMemory(mappedRegionA);
        kv->mapSharedMemory(mappedRegionB);
        auto byteRegionA = static_cast<uint8_t *>(mappedRegionA);
        auto byteRegionB = static_cast<uint8_t *>(mappedRegionB);

        // Update segments and check changes reflected
        uint8_t *full = kv->get();
        uint8_t *segmentA = kv->getSegment(1, 2);
        uint8_t *segmentB = kv->getSegment(4, 3);

        segmentA[1] = 8;
        segmentB[2] = 8;

        std::vector<uint8_t> expected = {0, 1, 8, 3, 4, 5, 8};
        for (int i = 0; i < 5; i++) {
            REQUIRE(full[i] == expected.at(i));
            REQUIRE(byteRegionA[i] == expected.at(i));
            REQUIRE(byteRegionB[i] == expected.at(i));
        }

        // Update shared regions and check reflected in segments
        byteRegionB[1] = 9;
        REQUIRE(segmentA[0] == 9);

        byteRegionA[5] = 1;
        REQUIRE(segmentB[1] == 1);
    }

    TEST_CASE("Test pulling in full async mode") {
        util::setEnvVar("FULL_ASYNC", "1");
        util::getSystemConfig().reset();

        auto kv = setupKV(6);
        REQUIRE(kv->empty());
        REQUIRE(kv->size() == 6);
        
        // Set up value in Redis
        redis::Redis &redisState = redis::Redis::getState();
        std::vector<uint8_t> value = {0, 1, 2, 3, 4, 5};
        redisState.set(kv->key, value);

        std::vector<uint8_t> expected;

        SECTION("Check pull ignored when async and full async") {
            // Pull and check storage is initialised
            kv->pull(true);
            REQUIRE(!kv->empty());
            REQUIRE(kv->size() == 6);
            expected = {0, 0, 0, 0, 0, 0};
        }

        SECTION("Check pull respected when not async but in full async mode") {
            // Pull and check storage is initialised
            kv->pull(false);
            REQUIRE(!kv->empty());
            REQUIRE(kv->size() == 6);
            expected = {0, 1, 2, 3, 4, 5};
        }

        uint8_t *actualBytes = kv->get();
        std::vector<uint8_t > actual(actualBytes, actualBytes + 6);
        REQUIRE(actual == expected);
        
        util::unsetEnvVar("FULL_ASYNC");
        util::getSystemConfig().reset();
    }
}