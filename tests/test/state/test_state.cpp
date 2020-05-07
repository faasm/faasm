#include <catch/catch.hpp>

#include "utils.h"

#include <redis/Redis.h>
#include <util/memory.h>
#include <util/config.h>
#include <state/State.h>
#include <sys/mman.h>
#include <emulator/emulator.h>
#include <faasm/state.h>
#include <util/state.h>

using namespace state;

namespace tests {
    static int staticCount = 0;
    static std::string originalStateMode;

    static void setUpStateMode(const std::string &newMode) {
        cleanSystem();

        util::SystemConfig &conf = util::getSystemConfig();
        originalStateMode = conf.stateMode;
        conf.stateMode = newMode;
    }

    static void resetStateMode() {
        util::getSystemConfig().stateMode = originalStateMode;
    }

    void setUpDummyServer(DummyStateServer &server, const std::vector<uint8_t> &values) {
        setUpStateMode("inmemory");

        staticCount++;
        const std::string stateKey = "state_key_" + std::to_string(staticCount);

        // Set state remotely
        server.dummyUser = getEmulatorUser();
        server.dummyKey = stateKey;
        server.dummyData = values;
    }

    std::shared_ptr<StateKeyValue> setupKV(size_t size) {
        // We have to make sure emulator is using the right user
        const std::string user = getEmulatorUser();

        staticCount++;
        const std::string stateKey = "state_key_" + std::to_string(staticCount);

        // Get state and remove key if already exists
        State &s = getGlobalState();
        auto kv = s.getKV(user, stateKey, size);

        return kv;
    }

    static void doStateSizeCheck(const std::string &stateMode) {
        setUpStateMode(stateMode);

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

    TEST_CASE("Test Redis state sizes", "[state]") {
        doStateSizeCheck("redis");
    }

    TEST_CASE("Test in-memory state sizes", "[state]") {
        doStateSizeCheck("inmemory");
    }

    static void doGetSetCheck(const std::string &stateMode) {
        setUpStateMode(stateMode);

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
        if (stateMode == "redis") {
            REQUIRE(redisState.get(actualKey).empty());
        }

        kv->pushFull();
        kv->get(actual.data());
        REQUIRE(actual == values);

        // Check that when pushed, the update is pushed to redis
        if (stateMode == "redis") {
            REQUIRE(redisState.get(actualKey) == values);
        }

        resetStateMode();
    }

    TEST_CASE("Test simple redis state get/set", "[state]") {
        doGetSetCheck("redis");
    }

    TEST_CASE("Test simple in memory state get/set", "[state]") {
        doGetSetCheck("inmemory");
    }

    TEST_CASE("Test in memory get/ set segment", "[state]") {
        DummyStateServer server;
        std::vector<uint8_t> values = {0, 0, 1, 1, 2, 2, 3, 3, 4, 4};
        setUpDummyServer(server, values);
        
        // Get, push, pull
        server.start(3);

        // Get locally
        std::vector<uint8_t> actual = server.getLocalKvValue();
        REQUIRE(actual == values);

        // Update a subsection
        std::vector<uint8_t> update = {8, 8, 8};
        std::shared_ptr<state::StateKeyValue> localKv = server.getLocalKv();
        localKv->setSegment(3, update.data(), 3);

        std::vector<uint8_t> expected = {0, 0, 1, 8, 8, 8, 3, 3, 4, 4};
        localKv->get(actual.data());
        REQUIRE(actual == expected);

        // Check remote is unchanged
        REQUIRE(server.getRemoteKvValue() == values);

        // Try getting a segment locally
        std::vector<uint8_t> actualSegment(3);
        localKv->getSegment(3, actualSegment.data(), 3);
        REQUIRE(actualSegment == update);

        // Run push and check remote is updated
        localKv->pushPartial();
        REQUIRE(server.getRemoteKvValue() == expected);

        // Wait for server to finish
        server.wait();

        resetStateMode();
    }

    TEST_CASE("Test in memory marking segments dirty", "[state]") {
        std::vector<uint8_t> values = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        DummyStateServer server;
        setUpDummyServer(server, values);

        // Get, push, pull
        server.start(3);
        
        // Get pointer to local and update in memory only
        const std::shared_ptr<state::StateKeyValue> &localKv = server.getLocalKv();
        uint8_t *ptr = localKv->get();
        ptr[0] = 8;
        ptr[5] = 7;

        // Mark one region as dirty and do push partial
        localKv->flagSegmentDirty(0, 2);
        localKv->pushPartial();

        // Update expectation
        values.at(0) = 8;

        // Check remote
        REQUIRE(server.getRemoteKvValue() == values);

        // Check local value has been set with the latest remote value
        std::vector<uint8_t> actualMemory(ptr, ptr + values.size());
        REQUIRE(actualMemory == values);

        server.wait();

        resetStateMode();
    }

    TEST_CASE("Test overlaps with multiple segments dirty", "[state]") {
        std::vector<uint8_t> values = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        DummyStateServer server;
        setUpDummyServer(server, values);

        // Get, push, pull
        server.start(3);
        
        // Get pointer to local data
        const std::shared_ptr<state::StateKeyValue> &localKv = server.getLocalKv();
        uint8_t *statePtr = localKv->get();

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
        localKv->flagSegmentDirty(1, 3);
        localKv->flagSegmentDirty(10, 2);
        localKv->flagSegmentDirty(14, 4);

        // Update one non-overlapping value remotely
        std::vector<uint8_t> directA = {2, 2};
        const std::shared_ptr<state::StateKeyValue> &remoteKv = server.getRemoteKv();
        remoteKv->setSegment(6, directA.data(), 2);

        // Update one overlapping value remotely
        std::vector<uint8_t> directB = {6, 6, 6, 6, 6};
        remoteKv->setSegment(0, directB.data(), 5);

        // Check expectations before push
        std::vector<uint8_t> expectedLocal = {0, 1, 2, 3, 0, 0, 0, 0, 0, 0, 4, 5, 0, 0, 7, 7, 7, 7, 0, 0};
        std::vector<uint8_t> expectedRemote = {6, 6, 6, 6, 6, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        REQUIRE(server.getLocalKvValue() == expectedLocal);
        REQUIRE(server.getRemoteKvValue() == expectedRemote);

        // Push changes
        localKv->pushPartial();

        // Check updates are made locally and remotely
        std::vector<uint8_t> expected = {6, 1, 2, 3, 6, 0, 2, 2, 0, 0, 4, 5, 0, 0, 7, 7, 7, 7, 0, 0};

        REQUIRE(server.getLocalKvValue() == expected);
        REQUIRE(server.getRemoteKvValue() == expected);

        server.wait();

        resetStateMode();
    }

    TEST_CASE("Test in memory partial update of doubles in state", "[state]") {
        long nDoubles = 20;
        long nBytes = nDoubles * sizeof(double);

        std::vector<uint8_t> values(nBytes, 0);
        DummyStateServer server;
        setUpDummyServer(server, values);

        // Get, push, pull
        server.start(3);
        
        // Set up both with zeroes initially
        std::vector<double> expected(nDoubles);
        std::vector<uint8_t> actualBytes(nBytes);
        memset(expected.data(), 0, nBytes);
        memset(actualBytes.data(), 0, nBytes);
        
        // Update a value locally and flag dirty
        const std::shared_ptr<state::StateKeyValue> &localKv = server.getLocalKv();
        auto actualPtr = reinterpret_cast<double *>(localKv->get());
        auto expectedPtr = expected.data();
        actualPtr[0] = 123.456;
        expectedPtr[0] = 123.456;
        localKv->flagSegmentDirty(0, sizeof(double));

        // Update another value
        actualPtr[1] = -100304.223;
        expectedPtr[1] = -100304.223;
        localKv->flagSegmentDirty(1 * sizeof(double), sizeof(double));

        // And another
        actualPtr[9] = 6090293.222;
        expectedPtr[9] = 6090293.222;
        localKv->flagSegmentDirty(9 * sizeof(double), sizeof(double));

        // And another
        actualPtr[13] = -123.444;
        expectedPtr[13] = -123.444;
        localKv->flagSegmentDirty(13 * sizeof(double), sizeof(double));

        // Push
        localKv->pushPartial();

        // Check local
        auto postPushDoublePtr = reinterpret_cast<double *>(localKv->get());
        std::vector<double> actualPostPush(postPushDoublePtr, postPushDoublePtr + nDoubles);
        REQUIRE(expected == actualPostPush);

        // Check remote
        std::vector<uint8_t> remoteValue = server.getRemoteKvValue();
        auto postPushDoublePtrRemote = reinterpret_cast<double *>(remoteValue.data());
        std::vector<double> actualPostPushRemote(postPushDoublePtr, postPushDoublePtr + nDoubles);
        REQUIRE(expected == actualPostPushRemote);

        server.wait();

        resetStateMode();
    }

    TEST_CASE("Test set segment cannot be out of bounds", "[state]") {
        auto kv = setupKV(2);

        // Set a segment offset
        std::vector<uint8_t> update = {8, 8, 8};

        REQUIRE_THROWS(kv->setSegment(5, update.data(), 3));
    }

    TEST_CASE("Test partially setting just first/ last element", "[state]") {
        std::vector<uint8_t> values = {0, 1, 2, 3, 4};
        DummyStateServer server;
        setUpDummyServer(server, values);

        // Only 3 push-partial messages as kv not fully allocated
        server.start(3);
        
        // Update just the last element
        std::vector<uint8_t> update = {8};
        const std::shared_ptr<state::StateKeyValue> &localKv = server.getLocalKv();
        localKv->setSegment(4, update.data(), 1);

        localKv->pushPartial();
        std::vector<uint8_t> expected = {0, 1, 2, 3, 8};
        REQUIRE(server.getRemoteKvValue() == expected);

        // Update the first
        localKv->setSegment(0, update.data(), 1);
        localKv->pushPartial();
        expected = {8, 1, 2, 3, 8};
        REQUIRE(server.getRemoteKvValue() == expected);

        // Update two
        update = {6};
        localKv->setSegment(0, update.data(), 1);
        localKv->setSegment(4, update.data(), 1);

        localKv->pushPartial();
        expected = {6, 1, 2, 3, 6};
        REQUIRE(server.getRemoteKvValue() == expected);

        server.wait();

        resetStateMode();
    }

    TEST_CASE("Test push partial with mask", "[state]") {
        size_t stateSize = 4 * sizeof(double);
        std::vector<uint8_t> values(stateSize, 0);
        DummyStateServer server;
        setUpDummyServer(server, values);

        // Get, full push, push partial
        server.start(4);
        
        // Create another local KV of same size
        State &state = getGlobalState();
        auto maskKv = state.getKV(getEmulatorUser(), "dummy_mask", stateSize);
        
        // Set up value locally
        const std::shared_ptr<state::StateKeyValue> &localKv = server.getLocalKv();
        uint8_t *dataBytePtr = localKv->get();
        auto dataDoublePtr = reinterpret_cast<double *>(dataBytePtr);
        std::vector<double> initial = {
                1.2345,
                12.345,
                987.6543,
                10987654.3
        };

        std::copy(initial.begin(), initial.end(), dataDoublePtr);

        // Push 
        localKv->flagDirty();
        localKv->pushFull();

        // Check pushed remotely
        std::vector<uint8_t> actualBytes = server.getRemoteKvValue();
        auto actualDoublePtr = reinterpret_cast<double *>(actualBytes.data());
        std::vector<double> actualDoubles(actualDoublePtr, actualDoublePtr + 4);

        REQUIRE(actualDoubles == initial);

        // Now update a couple of elements locally
        dataDoublePtr[1] = 11.11;
        dataDoublePtr[2] = 222.222;
        dataDoublePtr[3] = 3333.3333;

        // Mask two as having changed and push with mask
        uint8_t *maskBytePtr = maskKv->get();
        auto maskIntPtr = reinterpret_cast<unsigned int *>(maskBytePtr);
        faasm::maskDouble(maskIntPtr, 1);
        faasm::maskDouble(maskIntPtr, 3);

        localKv->flagDirty();
        localKv->pushPartialMask(maskKv);

        // Expected value will be a mix of new and old
        std::vector<double> expected = {
                1.2345,    // Old
                11.11,     // New (updated in memory and masked)
                987.6543,  // Old (updated in memory but not masked)
                3333.3333  // New (updated in memory and masked)
        };

        // Check remotely
        std::vector<uint8_t> actualValue2 = server.getRemoteKvValue();
        auto actualDoublesPtr = reinterpret_cast<double *>(actualValue2.data());
        std::vector<double> actualDoubles2(actualDoublesPtr, actualDoublesPtr + 4);
        REQUIRE(actualDoubles2 == expected);

        server.wait();

        resetStateMode();
    }

    void checkPulling(bool async) {
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
        if (!async) {
            kv->pull();
        }

        std::vector<uint8_t> actual(4);
        kv->get(actual.data());

        if (async) {
            REQUIRE(actual == values);
        } else {
            REQUIRE(actual == newValues);
        }

        resetStateMode();
    }

    TEST_CASE("Test redis async pulling", "[state]") {
        checkPulling(true);
    }

    TEST_CASE("Test redis sync pulling", "[state]") {
        checkPulling(false);
    }

    TEST_CASE("Test pushing only happens when dirty", "[state]") {
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
        kv->mapSharedMemory(mappedRegionA, 0, 2);

        void *mappedRegionB = mmap(nullptr, memSize, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        kv->mapSharedMemory(mappedRegionB, 0, 2);
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

    TEST_CASE("Test mapping shared memory pulls if not initialised", "[state]") {
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

        auto byteRegion = static_cast<uint8_t *>(mappedRegion);
        std::vector<uint8_t> actualValue(byteRegion, byteRegion + length);
        REQUIRE(actualValue == value);

        resetStateMode();
    }

    TEST_CASE("Test mapping small shared memory offsets", "[state]") {
        // Set up the KV
        auto kv = setupKV(7);
        std::vector<uint8_t> value = {0, 1, 2, 3, 4, 5, 6};
        kv->set(value.data());

        // Map a single page of host memory
        void *mappedRegionA = mmap(nullptr, util::HOST_PAGE_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        void *mappedRegionB = mmap(nullptr, util::HOST_PAGE_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        // Map them to small segments of the shared memory
        kv->mapSharedMemory(mappedRegionA, 0, 1);
        kv->mapSharedMemory(mappedRegionB, 0, 1);

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

    TEST_CASE("Test mapping bigger uninitialized shared memory offsets", "[state]") {
        setUpStateMode("redis");

        // Define some larger chunks
        size_t mappingSize = 3 * util::HOST_PAGE_SIZE;

        // Set up a larger total value
        size_t totalSize = (10 * util::HOST_PAGE_SIZE) + 15;
        auto kv = setupKV(totalSize);
        std::string actualKey = util::keyForUser(kv->user, kv->key);

        // Write ones to storage
        std::vector<uint8_t> value(totalSize);
        std::fill(value.data(), value.data() + totalSize, 1);
        redis::Redis &redisState = redis::Redis::getState();
        redisState.set(actualKey, value);

        // Map a couple of segments in host memory (as would be done by the wasm module)
        void *mappedRegionA = mmap(nullptr, mappingSize, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        void *mappedRegionB = mmap(nullptr, mappingSize, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        // Do the mapping and check they're reporting the correct offset
        kv->mapSharedMemory(mappedRegionA, 6, 3);
        kv->mapSharedMemory(mappedRegionB, 2, 3);

        // Get a byte pointer to each
        auto byteRegionA = static_cast<uint8_t *>(mappedRegionA);
        auto byteRegionB = static_cast<uint8_t *>(mappedRegionB);

        // Write something to each one
        byteRegionA[5] = 5;
        byteRegionB[9] = 9;

        // Get pointers to these segments
        size_t offsetA = (6 * util::HOST_PAGE_SIZE);
        size_t offsetB = (2 * util::HOST_PAGE_SIZE);
        uint8_t *segmentA = kv->getSegment(offsetA, 10);
        uint8_t *segmentB = kv->getSegment(offsetB, 10);

        REQUIRE(segmentA[0] == 1);
        REQUIRE(segmentB[0] == 1);
        REQUIRE(segmentA[5] == 5);
        REQUIRE(segmentB[9] == 9);

        resetStateMode();
    }

    TEST_CASE("Test pulling") {
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

    TEST_CASE("Test deletion", "[state]") {
        setUpStateMode("redis");

        redis::Redis &redisState = redis::Redis::getState();
        auto kv = setupKV(5);
        std::string actualKey = util::keyForUser(kv->user, kv->key);

        // Push some data and check
        std::vector<uint8_t> values = {0, 1, 2, 3, 4};
        kv->set(values.data());
        kv->pushFull();
        REQUIRE(redisState.get(actualKey) == values);

        kv->deleteGlobal();
        redisState.get(actualKey);

        resetStateMode();
    }

    TEST_CASE("Test appended state with KV", "[state]") {
        auto kv = setupKV(1);

        std::vector<uint8_t> valuesA = {0, 1, 2, 3, 4};
        std::vector<uint8_t> valuesB = {5, 6};
        std::vector<uint8_t> valuesC = {6, 5, 4, 3, 2, 1};

        // Append one
        kv->append(valuesA.data(), valuesA.size());

        // Check
        std::vector<uint8_t> actualA = {0, 0, 0, 0, 0};
        kv->getAppended(actualA.data(), actualA.size(), 1);
        REQUIRE(actualA == valuesA);

        // Append a few and check
        kv->append(valuesB.data(), valuesB.size());
        kv->append(valuesB.data(), valuesB.size());
        kv->append(valuesC.data(), valuesC.size());

        size_t actualSize = valuesA.size() + 2 * valuesB.size() + valuesC.size();
        std::vector<uint8_t> actualMulti(actualSize, 0);

        std::vector<uint8_t> expectedMulti = {0, 1, 2, 3, 4, 5, 6, 5, 6, 6, 5, 4, 3, 2, 1};
        kv->getAppended(actualMulti.data(), actualSize, 4);

        REQUIRE(actualMulti == expectedMulti);
    }
}