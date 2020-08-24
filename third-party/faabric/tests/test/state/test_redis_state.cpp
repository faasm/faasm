#include <catch/catch.hpp>

#include "faabric_utils.h"

#include <faabric/redis/Redis.h>
#include <faabric/util/memory.h>
#include <faabric/util/config.h>
#include <faabric/state/State.h>
#include <faasm/state.h>
#include <faabric/util/state.h>

#include <sys/mman.h>
#include <faabric/util/macros.h>

using namespace state;

/**
 * NOTE - there's some copy-pasting in here because we want to run
 * the same tests on in-memory and Redis versions.
 */

namespace tests {
    static int staticCount = 0;
    static std::string originalStateMode;

    static void setUpStateMode(const std::string &newMode) {
        cleanFaabric();

        util::SystemConfig &conf = util::getSystemConfig();
        originalStateMode = conf.stateMode;
        conf.stateMode = newMode;
    }

    static void resetStateMode() {
        util::getSystemConfig().stateMode = originalStateMode;
    }

    static std::shared_ptr<StateKeyValue> setupKV(size_t size) {
        // We have to make sure emulator is using the right user
        const std::string user = "demo";

        staticCount++;

        const std::string stateKey = "state_key_" + std::to_string(staticCount);

        // Get state and remove key if already exists
        State &s = getGlobalState();
        auto kv = s.getKV(user, stateKey, size);

        return kv;
    }

    TEST_CASE("Test Redis state sizes", "[state]") {
        setUpStateMode("redis");

        State &s = getGlobalState();
        std::string user = "alpha";
        std::string key = "beta";

        // Empty should be none
        size_t initialSize = s.getStateSize(user, key);
        REQUIRE(initialSize == 0);

        // Set a value
        std::vector<uint8_t> bytes = {0, 1, 2, 3, 4};
        auto kv = s.getKV(user, key, bytes.size());
        kv->set(bytes.data());
        kv->pushFull();

        // Get size
        REQUIRE(s.getStateSize(user, key) == bytes.size());

        resetStateMode();
    }

    TEST_CASE("Test simple redis state get/set", "[state]") {
        setUpStateMode("redis");

        redis::Redis &redisState = redis::Redis::getState();
        auto kv = setupKV(5);

        std::vector<uint8_t> actual(5);
        std::vector<uint8_t> expected = {0, 0, 0, 0, 0};

        kv->get(actual.data());
        REQUIRE(actual == expected);

        // Update
        std::vector<uint8_t> values = {0, 1, 2, 3, 4};
        kv->set(values.data());

        // Check that getting returns the update
        kv->get(actual.data());
        REQUIRE(actual == values);

        // Check that the underlying key in Redis isn't changed
        std::string actualKey = util::keyForUser(kv->user, kv->key);
        REQUIRE(redisState.get(actualKey).empty());

        kv->pushFull();
        kv->get(actual.data());
        REQUIRE(actual == values);

        // Check that when pushed, the update is pushed to redis
        REQUIRE(redisState.get(actualKey) == values);

        resetStateMode();
    }
    
    TEST_CASE("Test redis get/ set segment", "[state]") {
        setUpStateMode("redis");

        redis::Redis &redisState = redis::Redis::getState();
        auto kv = setupKV(10);

        // Set up and push
        std::vector<uint8_t> values = {0, 0, 1, 1, 2, 2, 3, 3, 4, 4};
        kv->set(values.data());
        kv->pushFull();

        // Get and check
        std::vector<uint8_t> actual(10);
        kv->get(actual.data());
        std::string actualKey = util::keyForUser(kv->user, kv->key);
        REQUIRE(actual == values);

            REQUIRE(redisState.get(actualKey) == values);

        // Update a subsection
        std::vector<uint8_t> update = {8, 8, 8};
        kv->setChunk(3, update.data(), 3);

        std::vector<uint8_t> expected = {0, 0, 1, 8, 8, 8, 3, 3, 4, 4};
        kv->get(actual.data());
        REQUIRE(actual == expected);

        // Check changed locally but not in redis
            REQUIRE(redisState.get(actualKey) == values);

        // Try getting a segment
        std::vector<uint8_t> actualSegment(3);
        kv->getChunk(3, actualSegment.data(), 3);
        REQUIRE(actualSegment == update);

        // Run push and check redis updated
        kv->pushPartial();
            REQUIRE(redisState.get(actualKey) == expected);

        resetStateMode();
    }

    TEST_CASE("Test redis pulls segment even when already allocated", "[state]") {
        setUpStateMode("redis");

        std::vector<uint8_t> values = {0, 0, 1, 1, 2, 2, 3, 3, 4, 4};
        
        // Set up a value in Redis
        redis::Redis &redisState = redis::Redis::getState();
        auto kv = setupKV(values.size());
        kv->set(values.data());
        kv->pushFull();

        std::string user = kv->user;
        std::string key = kv->key;
        
        // Clear locally
        State &globalState = state::getGlobalState();
        globalState.forceClearAll(false);
        
        // Check it's in Redis
        std::string actualKey = util::keyForUser(user, key);
        REQUIRE(redisState.get(actualKey) == values);
        
        // Access a chunk and check it's implicitly pulled
        auto kvAfter = globalState.getKV(user, key, values.size());
        std::vector<uint8_t> actualChunk(3, 0);
        std::vector<uint8_t> expectedChunk = {2, 2, 3};
        kvAfter->getChunk(4, actualChunk.data(), actualChunk.size());

        REQUIRE(actualChunk == expectedChunk);

        resetStateMode();
    }

    TEST_CASE("Test redis marking segments dirty", "[state]") {
        setUpStateMode("redis");

        redis::Redis &redisState = redis::Redis::getState();
        auto kv = setupKV(10);

        // Set up and push
        std::vector<uint8_t> values = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        kv->set(values.data());
        kv->pushFull();

        // Get pointer and update in memory only
        uint8_t *ptr = kv->get();
        ptr[0] = 8;
        ptr[5] = 7;

        // Mark one region as dirty and do push partial
        kv->flagChunkDirty(0, 2);
        kv->pushPartial();

        // Update expectation
        values.at(0) = 8;
        std::string actualKey = util::keyForUser(kv->user, kv->key);

        // Check in redis
            REQUIRE(redisState.get(actualKey) == values);

        // Check expectation
        std::vector<uint8_t> actualMemory(ptr, ptr + values.size());
        REQUIRE(actualMemory == values);

        resetStateMode();
    }

    TEST_CASE("Test redis overlaps with multiple segments dirty", "[state]") {
        setUpStateMode("redis");

        redis::Redis &redisState = redis::Redis::getState();
        auto kv = setupKV(20);
        std::string actualKey = util::keyForUser(kv->user, kv->key);
        const char *key = actualKey.c_str();

        // Set up and push
        std::vector<uint8_t> values = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        kv->set(values.data());
        kv->pushFull();

        // Get pointer
        uint8_t *statePtr = kv->get();

        // Update a couple of areas
        statePtr[1] = 1;
        statePtr[2] = 2;
        statePtr[3] = 3;

        statePtr[10] = 4;
        statePtr[11] = 5;

        statePtr[14] = 7;
        statePtr[15] = 7;
        statePtr[16] = 7;
        statePtr[17] = 7;

        // Mark regions as dirty
        kv->flagChunkDirty(1, 3);
        kv->flagChunkDirty(10, 2);
        kv->flagChunkDirty(14, 4);

        // Update one non-overlapping value in state
        std::vector<uint8_t> directA = {2, 2};
        redisState.setRange(key, 6, directA.data(), 2);

        // Update one overlapping value in state
        std::vector<uint8_t> directB = {6, 6, 6, 6, 6};
        redisState.setRange(key, 0, directB.data(), 5);

        // Check all updates are taken and the state ones take precedence
        std::vector<uint8_t> expected = {6, 1, 2, 3, 6, 0, 2, 2, 0, 0, 4, 5, 0, 0, 7, 7, 7, 7, 0, 0};

        // Push and check that with no pull we're up to date
        kv->pushPartial();
        REQUIRE(redisState.get(key) == expected);

        resetStateMode();
    }

    TEST_CASE("Test redis partial update of doubles in state", "[state]") {
        setUpStateMode("redis");

        redis::Redis &redisState = redis::Redis::getState();

        long nDoubles = 20;
        long nBytes = nDoubles * sizeof(double);
        auto kv = setupKV(nBytes);
        std::string actualKey = util::keyForUser(kv->user, kv->key);
        const char *key = actualKey.c_str();

        // Set up both with zeroes initiall
        std::vector<double> expected(nDoubles);
        std::vector<uint8_t> actualBytes(nBytes);
        memset(expected.data(), 0, nBytes);
        memset(actualBytes.data(), 0, nBytes);

        // Push zeroes to state
        kv->set(actualBytes.data());
        kv->pushFull();

        // Update some elements in both and flag dirty
        auto actualPtr = reinterpret_cast<double *>(kv->get());
        auto expectedPtr = expected.data();
        actualPtr[0] = 123.456;
        expectedPtr[0] = 123.456;
        kv->flagChunkDirty(0, sizeof(double));

        actualPtr[1] = -100304.223;
        expectedPtr[1] = -100304.223;
        kv->flagChunkDirty(1 * sizeof(double), sizeof(double));

        actualPtr[9] = 6090293.222;
        expectedPtr[9] = 6090293.222;
        kv->flagChunkDirty(9 * sizeof(double), sizeof(double));

        actualPtr[13] = -123.444;
        expectedPtr[13] = -123.444;
        kv->flagChunkDirty(13 * sizeof(double), sizeof(double));

        // Push and check that with no pull we're up to date
        kv->pushPartial();
        auto postPushDoublePtr = reinterpret_cast<double *>(kv->get());
        std::vector<double> actualPostPush(postPushDoublePtr, postPushDoublePtr + nDoubles);
        REQUIRE(expected == actualPostPush);

        std::vector<double> actualFromRedis(nDoubles);
        redisState.get(key, BYTES(actualFromRedis.data()), nBytes);
        REQUIRE(expected == actualFromRedis);

        resetStateMode();
    }

    TEST_CASE("Test redis partially setting just first/ last element", "[state]") {
        setUpStateMode("redis");

        redis::Redis &redisState = redis::Redis::getState();
        auto kv = setupKV(5);
        std::string actualKey = util::keyForUser(kv->user, kv->key);

        // Set up and push
        std::vector<uint8_t> values = {0, 1, 2, 3, 4};
        kv->set(values.data());
        kv->pushFull();

        // Update just the last
        std::vector<uint8_t> update = {8};
        kv->setChunk(4, update.data(), 1);

        kv->pushPartial();
        std::vector<uint8_t> expected = {0, 1, 2, 3, 8};
        REQUIRE(redisState.get(actualKey) == expected);

        // Update the first
        kv->setChunk(0, update.data(), 1);

        kv->pushPartial();
        expected = {8, 1, 2, 3, 8};
        REQUIRE(redisState.get(actualKey) == expected);

        // Update both
        update = {6};
        kv->setChunk(0, update.data(), 1);
        kv->setChunk(4, update.data(), 1);

        kv->pushPartial();
        expected = {6, 1, 2, 3, 6};
        REQUIRE(redisState.get(actualKey) == expected);

        resetStateMode();
    }

    TEST_CASE("Test redis push partial with mask", "[state]") {
        setUpStateMode("redis");

        redis::Redis &redisState = redis::Redis::getState();

        // Create two key-values of same size
        size_t stateSize = 4 * sizeof(double);
        std::shared_ptr<StateKeyValue> kvData = setupKV(stateSize);
        std::shared_ptr<StateKeyValue> kvMask = setupKV(stateSize);

        // Set up value in memory
        uint8_t *dataBytePtr = kvData->get();
        auto dataDoublePtr = reinterpret_cast<double *>(dataBytePtr);
        std::vector<double> initial = {
                1.2345,
                12.345,
                987.6543,
                10987654.3
        };

        std::copy(initial.begin(), initial.end(), dataDoublePtr);

        // Push 
        kvData->flagDirty();
        kvData->pushFull();

        // Check round trip
        std::string actualKey = util::keyForUser(kvData->user, kvData->key);
        std::vector<uint8_t> actualBytes = redisState.get(actualKey);
        auto actualDoublePtr = reinterpret_cast<double*>(actualBytes.data());
        std::vector<double> actualDoubles(actualDoublePtr, actualDoublePtr + 4);

        REQUIRE(actualDoubles == initial);

        // Now update a couple of elements in memory
        dataDoublePtr[1] = 11.11;
        dataDoublePtr[2] = 222.222;
        dataDoublePtr[3] = 3333.3333;

        // Mask two as having changed and push with mask
        uint8_t *maskBytePtr = kvMask->get();
        auto maskIntPtr = reinterpret_cast<unsigned int *>(maskBytePtr);
        faasm::maskDouble(maskIntPtr, 1);
        faasm::maskDouble(maskIntPtr, 3);

        kvData->flagDirty();
        kvData->pushPartialMask(kvMask);

        // Expected value will be a mix of new and old
        std::vector<double> expected = {
                1.2345,    // Old
                11.11,     // New (updated in memory and masked)
                987.6543,  // Old (updated in memory but not masked)
                3333.3333  // New (updated in memory and masked)
        };

        // Check in redis
        std::vector<uint8_t> actualValue2 = redisState.get(actualKey);
        auto actualDoublesPtr = reinterpret_cast<double *>(actualValue2.data());
        std::vector<double> actualDoubles2(actualDoublesPtr, actualDoublesPtr + 4);
        REQUIRE(actualDoubles2 == expected);

        resetStateMode();
    }


    TEST_CASE("Test redis async pulling", "[state]") {
        setUpStateMode("redis");

        auto kv = setupKV(4);
        std::string actualKey = util::keyForUser(kv->user, kv->key);

        std::vector<uint8_t> values = {0, 1, 2, 3};

        // Push and make sure reflected in redis
        kv->set(values.data());
        kv->pushFull();

        redis::Redis &redisState = redis::Redis::getState();
        REQUIRE(redisState.get(actualKey) == values);

        // Now update in Redis directly
        std::vector<uint8_t> newValues = {5, 5, 5, 5};
        redisState.set(actualKey, newValues);

        // Get and check whether the remote is pulled
        std::vector<uint8_t> actual(4);
        kv->get(actual.data());
        REQUIRE(actual == values);

        resetStateMode();
    }

    TEST_CASE("Test redis pushing only happens when dirty", "[state]") {
        setUpStateMode("redis");

        redis::Redis &redisState = redis::Redis::getState();

        auto kv = setupKV(4);
        std::string actualKey = util::keyForUser(kv->user, kv->key);

        std::vector<uint8_t> values = {0, 1, 2, 3};
        kv->set(values.data());
        kv->pushFull();

        // Change in redis directly
        std::vector<uint8_t> newValues = {3, 4, 5, 6};
        redisState.set(actualKey, newValues);

        // Push and make sure redis not changed as it's not dirty
        kv->pushFull();
        REQUIRE(redisState.get(actualKey) == newValues);

        // Now change locally and check push happens
        std::vector<uint8_t> newValues2 = {7, 7, 7, 7};
        kv->set(newValues2.data());
        kv->pushFull();
        REQUIRE(redisState.get(actualKey) == newValues2);

        resetStateMode();
    }

    TEST_CASE("Test redis mapping shared memory does not pull if not initialised", "[state]") {
        setUpStateMode("redis");

        // Set up the KV
        int length = 5;
        auto kv = setupKV(length);
        std::string actualKey = util::keyForUser(kv->user, kv->key);

        // Write value direct to redis
        std::vector<uint8_t> value = {0, 1, 2, 3, 4};
        redis::Redis &redisState = redis::Redis::getState();
        redisState.set(actualKey, value.data(), length);

        // Try to map the kv
        void *mappedRegion = mmap(nullptr, util::HOST_PAGE_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        kv->mapSharedMemory(mappedRegion, 0, 1);

        // Get to ensure the value is present
        kv->get();

        // Check
        auto byteRegion = static_cast<uint8_t *>(mappedRegion);
        std::vector<uint8_t> actualValue(byteRegion, byteRegion + length);
        REQUIRE(actualValue == value);

        resetStateMode();
    }

    TEST_CASE("Test redis state pulling") {
        setUpStateMode("redis");

        auto kv = setupKV(6);
        REQUIRE(kv->size() == 6);
        std::string actualKey = util::keyForUser(kv->user, kv->key);

        // Set up value in Redis
        redis::Redis &redisState = redis::Redis::getState();
        std::vector<uint8_t> value = {0, 1, 2, 3, 4, 5};
        redisState.set(actualKey, value);

        std::vector<uint8_t> expected;

        // Pull and check storage is initialised
        kv->pull();
        REQUIRE(kv->size() == 6);

        expected = {0, 1, 2, 3, 4, 5};
        uint8_t *actualBytes = kv->get();
        std::vector<uint8_t> actual(actualBytes, actualBytes + 6);
        REQUIRE(actual == expected);

        resetStateMode();
    }

    TEST_CASE("Test redis state deletion", "[state]") {
        setUpStateMode("redis");

        redis::Redis &redisState = redis::Redis::getState();
        auto kv = setupKV(5);
        std::string actualKey = util::keyForUser(kv->user, kv->key);

        // Push some data and check
        std::vector<uint8_t> values = {0, 1, 2, 3, 4};
        kv->set(values.data());
        kv->pushFull();
        REQUIRE(redisState.get(actualKey) == values);

        state::getGlobalState().deleteKV(kv->user, kv->key);
        REQUIRE(redisState.get(actualKey).size() == 0);

        resetStateMode();
    }
}