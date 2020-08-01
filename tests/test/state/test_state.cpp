#include <catch/catch.hpp>

#include "utils.h"

#include <redis/Redis.h>
#include <util/memory.h>
#include <util/config.h>
#include <state/State.h>
#include <sys/mman.h>
#include <emulator/emulator.h>
#include <faasm/state.h>
#include <state/InMemoryStateKeyValue.h>

using namespace state;

namespace tests {
    static int staticCount = 0;

    static void setUpDummyServer(DummyStateServer &server, const std::vector<uint8_t> &values) {
        staticCount++;
        const std::string stateKey = "state_key_" + std::to_string(staticCount);

        // Set state remotely
        server.dummyUser = getEmulatorUser();
        server.dummyKey = stateKey;
        server.dummyData = values;
    }

    static std::shared_ptr<StateKeyValue> setupKV(size_t size) {
        // We have to make sure emulator is using the right user
        const std::string user = getEmulatorUser();

        staticCount++;
        const std::string stateKey = "state_key_" + std::to_string(staticCount);

        // Get state and remove key if already exists
        State &s = getGlobalState();
        auto kv = s.getKV(user, stateKey, size);

        return kv;
    }

    TEST_CASE("Test in-memory state sizes", "[state]") {
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
    }

    TEST_CASE("Test simple in memory state get/set", "[state]") {
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

        kv->pushFull();
        kv->get(actual.data());
        REQUIRE(actual == values);
    }

    TEST_CASE("Test in memory get/ set chunk", "[state]") {
        DummyStateServer server;
        std::vector<uint8_t> values = {0, 0, 1, 1, 2, 2, 3, 3, 4, 4};
        setUpDummyServer(server, values);

        // Get, push, pull
        server.start();

        // Get locally
        std::vector<uint8_t> actual = server.getLocalKvValue();
        REQUIRE(actual == values);

        // Update a subsection
        std::vector<uint8_t> update = {8, 8, 8};
        std::shared_ptr<state::StateKeyValue> localKv = server.getLocalKv();
        localKv->setChunk(3, update.data(), 3);

        std::vector<uint8_t> expected = {0, 0, 1, 8, 8, 8, 3, 3, 4, 4};
        localKv->get(actual.data());
        REQUIRE(actual == expected);

        // Check remote is unchanged
        REQUIRE(server.getRemoteKvValue() == values);

        // Try getting a chunk locally
        std::vector<uint8_t> actualChunk(3);
        localKv->getChunk(3, actualChunk.data(), 3);
        REQUIRE(actualChunk == update);

        // Run push and check remote is updated
        localKv->pushPartial();
        REQUIRE(server.getRemoteKvValue() == expected);

        // Wait for server to finish
        DummyStateServer::stop();
        server.wait();
    }

    TEST_CASE("Test in memory marking chunks dirty", "[state]") {
        std::vector<uint8_t> values = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        DummyStateServer server;
        setUpDummyServer(server, values);

        // Get, push, pull
        server.start();

        // Get pointer to local and update in memory only
        const std::shared_ptr<state::StateKeyValue> &localKv = server.getLocalKv();
        uint8_t *ptr = localKv->get();
        ptr[0] = 8;
        ptr[5] = 7;

        // Mark one region as dirty and do push partial
        localKv->flagChunkDirty(0, 2);
        localKv->pushPartial();

        // Update expectation
        values.at(0) = 8;

        // Check remote
        REQUIRE(server.getRemoteKvValue() == values);

        // Check local value has been set with the latest remote value
        std::vector<uint8_t> actualMemory(ptr, ptr + values.size());
        REQUIRE(actualMemory == values);

        DummyStateServer::stop();
        server.wait();
    }

    TEST_CASE("Test overlaps with multiple chunks dirty", "[state]") {
        std::vector<uint8_t> values = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        DummyStateServer server;
        setUpDummyServer(server, values);

        // Get, push, pull
        server.start();

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
        localKv->flagChunkDirty(1, 3);
        localKv->flagChunkDirty(10, 2);
        localKv->flagChunkDirty(14, 4);

        // Update one non-overlapping value remotely
        std::vector<uint8_t> directA = {2, 2};
        const std::shared_ptr<state::StateKeyValue> &remoteKv = server.getRemoteKv();
        remoteKv->setChunk(6, directA.data(), 2);

        // Update one overlapping value remotely
        std::vector<uint8_t> directB = {6, 6, 6, 6, 6};
        remoteKv->setChunk(0, directB.data(), 5);

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

        DummyStateServer::stop();
        server.wait();
    }

    TEST_CASE("Test in memory partial update of doubles in state", "[state]") {
        long nDoubles = 20;
        long nBytes = nDoubles * sizeof(double);

        std::vector<uint8_t> values(nBytes, 0);
        DummyStateServer server;
        setUpDummyServer(server, values);

        // Get, push, pull
        server.start();

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
        localKv->flagChunkDirty(0, sizeof(double));

        // Update another value
        actualPtr[1] = -100304.223;
        expectedPtr[1] = -100304.223;
        localKv->flagChunkDirty(1 * sizeof(double), sizeof(double));

        // And another
        actualPtr[9] = 6090293.222;
        expectedPtr[9] = 6090293.222;
        localKv->flagChunkDirty(9 * sizeof(double), sizeof(double));

        // And another
        actualPtr[13] = -123.444;
        expectedPtr[13] = -123.444;
        localKv->flagChunkDirty(13 * sizeof(double), sizeof(double));

        // Push
        localKv->pushPartial();

        // Check local
        auto postPushDoublePtr = reinterpret_cast<double *>(localKv->get());
        std::vector<double> actualPostPush(postPushDoublePtr, postPushDoublePtr + nDoubles);
        REQUIRE(expected == actualPostPush);

        // Check remote
        std::vector<uint8_t> remoteValue = server.getRemoteKvValue();
        std::vector<double> actualPostPushRemote(postPushDoublePtr, postPushDoublePtr + nDoubles);
        REQUIRE(expected == actualPostPushRemote);

        DummyStateServer::stop();
        server.wait();
    }

    TEST_CASE("Test set chunk cannot be over the size of the allocated memory", "[state]") {
        auto kv = setupKV(2);

        // Set a chunk offset
        std::vector<uint8_t> update = {8, 8, 8};

        long offset = util::HOST_PAGE_SIZE - 2;
        REQUIRE_THROWS(kv->setChunk(offset, update.data(), 3));
    }

    TEST_CASE("Test partially setting just first/ last element", "[state]") {
        std::vector<uint8_t> values = {0, 1, 2, 3, 4};
        DummyStateServer server;
        setUpDummyServer(server, values);

        // Only 3 push-partial messages as kv not fully allocated
        server.start();

        // Update just the last element
        std::vector<uint8_t> update = {8};
        const std::shared_ptr<state::StateKeyValue> &localKv = server.getLocalKv();
        localKv->setChunk(4, update.data(), 1);

        localKv->pushPartial();
        std::vector<uint8_t> expected = {0, 1, 2, 3, 8};
        REQUIRE(server.getRemoteKvValue() == expected);

        // Update the first
        localKv->setChunk(0, update.data(), 1);
        localKv->pushPartial();
        expected = {8, 1, 2, 3, 8};
        REQUIRE(server.getRemoteKvValue() == expected);

        // Update two
        update = {6};
        localKv->setChunk(0, update.data(), 1);
        localKv->setChunk(4, update.data(), 1);

        localKv->pushPartial();
        expected = {6, 1, 2, 3, 6};
        REQUIRE(server.getRemoteKvValue() == expected);

        DummyStateServer::stop();
        server.wait();
    }

    TEST_CASE("Test push partial with mask", "[state]") {
        size_t stateSize = 4 * sizeof(double);
        std::vector<uint8_t> values(stateSize, 0);
        DummyStateServer server;
        setUpDummyServer(server, values);

        // Get, full push, push partial
        server.start();

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

        DummyStateServer::stop();
        server.wait();
    }

    void checkPulling(bool doPull) {
        std::vector<uint8_t> values = {0, 1, 2, 3};
        std::vector<uint8_t> actual(values.size(), 0);

        DummyStateServer server;
        setUpDummyServer(server, values);

        // Get, with optional pull
        int nMessages = 1;
        if (doPull) {
            nMessages = 2;
        }

        server.start();

        // Initial pull
        const std::shared_ptr<state::StateKeyValue> &localKv = server.getLocalKv();
        localKv->pull();

        // Update directly on the remote KV
        std::shared_ptr<state::StateKeyValue> remoteKv = server.getRemoteKv();
        std::vector<uint8_t> newValues = {5, 5, 5, 5};
        remoteKv->set(newValues.data());

        if (doPull) {
            // Check locak changed with another pull
            localKv->pull();
            localKv->get(actual.data());
            REQUIRE(actual == newValues);
        } else {
            // Check local unchanged without another pull
            localKv->get(actual.data());
            REQUIRE(actual == values);
        }

        DummyStateServer::stop();
        server.wait();
    }

    TEST_CASE("Test updates pulled from remote", "[state]") {
        checkPulling(true);
    }

    TEST_CASE("Test updates not pulled from remote without call to pull", "[state]") {
        checkPulling(false);
    }

    TEST_CASE("Test pushing only happens when dirty", "[state]") {
        std::vector<uint8_t> values = {0, 1, 2, 3};
        DummyStateServer server;
        setUpDummyServer(server, values);

        server.start();

        // Pull locally
        const std::shared_ptr<state::StateKeyValue> &localKv = server.getLocalKv();
        localKv->pull();

        // Change remote directly
        std::shared_ptr<state::StateKeyValue> remoteKv = server.getRemoteKv();
        std::vector<uint8_t> newValues = {3, 4, 5, 6};
        remoteKv->set(newValues.data());

        // Push and make sure remote has not changed without local being dirty
        localKv->pushFull();
        REQUIRE(server.getRemoteKvValue() == newValues);

        // Now change locally and check push happens
        std::vector<uint8_t> newValues2 = {7, 7, 7, 7};
        localKv->set(newValues2.data());
        localKv->pushFull();
        REQUIRE(server.getRemoteKvValue() == newValues2);

        DummyStateServer::stop();
        server.wait();
    }

    TEST_CASE("Test mapping shared memory", "[state]") {
        cleanSystem();

        // Set up the KV
        State &s = getGlobalState();
        auto kv = s.getKV(getEmulatorUser(), "mapping_test", 5);
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

    TEST_CASE("Test mapping shared memory does not pull", "[state]") {
        std::vector<uint8_t> values = {0, 1, 2, 3, 4};
        std::vector<uint8_t> zeroes(values.size(), 0);
        DummyStateServer server;
        setUpDummyServer(server, values);

        // One implicit pull
        server.start();

        // Write value to remote
        const std::shared_ptr<state::StateKeyValue> &remoteKv = server.getRemoteKv();
        remoteKv->set(values.data());

        // Map the KV locally
        void *mappedRegion = mmap(nullptr, util::HOST_PAGE_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        const std::shared_ptr<state::StateKeyValue> &localKv = server.getLocalKv();
        localKv->mapSharedMemory(mappedRegion, 0, 1);

        // Check it's zeroed
        auto byteRegion = static_cast<uint8_t *>(mappedRegion);
        std::vector<uint8_t> actualValue(byteRegion, byteRegion + values.size());
        REQUIRE(actualValue == zeroes);

        // Now get the value and check it's implicitly pulled
        localKv->get();
        std::vector<uint8_t> actualValueAfterGet(byteRegion, byteRegion + values.size());
        REQUIRE(actualValueAfterGet == values);

        DummyStateServer::stop();
        server.wait();
    }

    TEST_CASE("Test mapping small shared memory offsets", "[state]") {
        cleanSystem();

        // Set up the KV
        std::vector<uint8_t> values = {0, 1, 2, 3, 4, 5, 6};
        State &s = getGlobalState();
        auto kv = s.getKV(getEmulatorUser(), "mapping_small_test", values.size());
        kv->set(values.data());

        // Map a single page of host memory
        void *mappedRegionA = mmap(nullptr, util::HOST_PAGE_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        void *mappedRegionB = mmap(nullptr, util::HOST_PAGE_SIZE, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        // Map them to small chunks of the shared memory
        kv->mapSharedMemory(mappedRegionA, 0, 1);
        kv->mapSharedMemory(mappedRegionB, 0, 1);

        auto byteRegionA = static_cast<uint8_t *>(mappedRegionA);
        auto byteRegionB = static_cast<uint8_t *>(mappedRegionB);

        // Update chunks and check changes reflected
        uint8_t *full = kv->get();
        uint8_t *chunkA = kv->getChunk(1, 2);
        uint8_t *chunkB = kv->getChunk(4, 3);

        chunkA[1] = 8;
        chunkB[2] = 8;

        std::vector<uint8_t> expected = {0, 1, 8, 3, 4, 5, 8};
        for (int i = 0; i < 5; i++) {
            REQUIRE(full[i] == expected.at(i));
            REQUIRE(byteRegionA[i] == expected.at(i));
            REQUIRE(byteRegionB[i] == expected.at(i));
        }

        // Update shared regions and check reflected in chunks
        byteRegionB[1] = 9;
        REQUIRE(chunkA[0] == 9);

        byteRegionA[5] = 1;
        REQUIRE(chunkB[1] == 1);
    }

    TEST_CASE("Test mapping bigger uninitialized shared memory offsets", "[state]") {
        // Define some mapping larger than a page
        size_t mappingSize = 3 * util::HOST_PAGE_SIZE;

        // Set up a larger total value full of ones
        size_t totalSize = (10 * util::HOST_PAGE_SIZE) + 15;
        std::vector<uint8_t> values(totalSize, 1);

        // Set up remote server
        DummyStateServer server;
        setUpDummyServer(server, values);

        // Expecting two implicit pulls
        server.start();

        // Map a couple of chunks in host memory (as would be done by the wasm module)
        void *mappedRegionA = mmap(nullptr, mappingSize, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        void *mappedRegionB = mmap(nullptr, mappingSize, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        // Do the mapping
        const std::shared_ptr<state::StateKeyValue> &localKv = server.getLocalKv();
        localKv->mapSharedMemory(mappedRegionA, 6, 3);
        localKv->mapSharedMemory(mappedRegionB, 2, 3);

        // Get a byte pointer to each
        auto byteRegionA = static_cast<uint8_t *>(mappedRegionA);
        auto byteRegionB = static_cast<uint8_t *>(mappedRegionB);

        // Get direct pointers to these chunks (will implicitly pull)
        size_t offsetA = (6 * util::HOST_PAGE_SIZE);
        size_t offsetB = (2 * util::HOST_PAGE_SIZE);
        uint8_t *chunkA = localKv->getChunk(offsetA, 10);
        uint8_t *chunkB = localKv->getChunk(offsetB, 10);

        // Write something to each one
        byteRegionA[5] = 5;
        byteRegionB[9] = 9;

        REQUIRE(chunkA[0] == 1);
        REQUIRE(chunkB[0] == 1);
        REQUIRE(chunkA[5] == 5);
        REQUIRE(chunkB[9] == 9);

        DummyStateServer::stop();
        server.wait();
    }

    TEST_CASE("Test deletion", "[state]") {
        std::vector<uint8_t> values = {0, 1, 2, 3, 4};
        DummyStateServer server;
        setUpDummyServer(server, values);

        // One pull, one deletion
        server.start();

        // Check data remotely and locally
        REQUIRE(server.getLocalKvValue() == values);
        REQUIRE(server.getRemoteKvValue() == values);

        // Delete from state
        getGlobalState().deleteKV(server.dummyUser, server.dummyKey);

        // Check it's gone
        REQUIRE(server.remoteState.getKVCount() == 0);

        DummyStateServer::stop();
        server.wait();
    }

    TEST_CASE("Test appended state with KV", "[state]") {
        cleanSystem();

        // Set up the KV
        State &s = getGlobalState();
        std::shared_ptr<StateKeyValue> kv;

        SECTION("Sizeless") {
            kv = s.getKV(getEmulatorUser(), "appending_test");
        }

        SECTION("With size") {
            std::vector<uint8_t> values(1, 0);
            kv = s.getKV(getEmulatorUser(), "appending_test", values.size());
            kv->set(values.data());
        }

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

        // Clear, then check again
        kv->clearAppended();
        kv->append(valuesB.data(), valuesB.size());
        kv->append(valuesC.data(), valuesC.size());

        size_t afterClearSize = valuesB.size() + valuesC.size();
        std::vector<uint8_t> actualAfterClear(afterClearSize, 0);
        std::vector<uint8_t> expectedAfterClear = {5, 6, 6, 5, 4, 3, 2, 1};
        kv->getAppended(actualAfterClear.data(), afterClearSize, 2);

        REQUIRE(actualAfterClear == expectedAfterClear);
    }

    TEST_CASE("Test remote appended state", "[state]") {
        DummyStateServer server;
        std::vector<uint8_t> empty;
        setUpDummyServer(server, empty);

        // One appends, two retrievals, one clear
        server.start();

        std::vector<uint8_t> valuesA = {0, 1, 2, 3, 4};
        std::vector<uint8_t> valuesB = {3, 3, 5, 5};

        // Append some data remotely
        const std::shared_ptr<state::StateKeyValue> &remoteKv = server.getRemoteKv();
        remoteKv->append(valuesA.data(), valuesA.size());

        // Append locally
        const std::shared_ptr<state::StateKeyValue> &localKv = server.getLocalKv();
        auto localInMemKv = std::static_pointer_cast<InMemoryStateKeyValue>(localKv);
        REQUIRE(!localInMemKv->isMaster());
        localKv->append(valuesB.data(), valuesB.size());

        // Check data is pushed remotely
        std::vector<uint8_t> expectedRemote = {0, 1, 2, 3, 4, 3, 3, 5, 5};
        std::vector<uint8_t> actualRemote(expectedRemote.size(), 0);
        remoteKv->getAppended(actualRemote.data(), actualRemote.size(), 2);
        REQUIRE(actualRemote == expectedRemote);

        // Append more data remotely
        remoteKv->append(valuesA.data(), valuesA.size());

        // Check locally
        std::vector<uint8_t> expectedLocal = {0, 1, 2, 3, 4, 3, 3, 5, 5, 0, 1, 2, 3, 4};
        std::vector<uint8_t> actualLocal(expectedLocal.size(), 0);
        localKv->getAppended(actualLocal.data(), actualLocal.size(), 3);
        REQUIRE(actualLocal == expectedLocal);

        // Clear and check again
        localKv->clearAppended();
        remoteKv->append(valuesB.data(), valuesB.size());
        std::vector<uint8_t> actualLocalAfterClear(valuesB.size(), 0);
        localKv->getAppended(actualLocalAfterClear.data(), actualLocalAfterClear.size(), 1);
        REQUIRE(actualLocalAfterClear == valuesB);

        DummyStateServer::stop();
        server.wait();
    }

    TEST_CASE("Test pushing pulling large state", "[state]") {
        size_t valueSize = (1024 * 1024 * 1024) + 123;
        std::vector<uint8_t> valuesA(valueSize, 1);
        std::vector<uint8_t> valuesB(valueSize, 2);

        DummyStateServer server;
        setUpDummyServer(server, valuesA);

        // One pull, one push
        server.start();

        // Pull locally
        const std::shared_ptr<state::StateKeyValue> &localKv = server.getLocalKv();
        localKv->pull();

        // Check equality
        const std::vector<uint8_t> &actualLocal = server.getLocalKvValue();
        REQUIRE(actualLocal == valuesA);

        // Push
        localKv->set(valuesB.data());
        localKv->pushFull();

        // Check equality of remote
        const std::vector<uint8_t> &actualRemote = server.getRemoteKvValue();
        REQUIRE(actualRemote == valuesB);

        DummyStateServer::stop();
        server.wait();
    }

    TEST_CASE("Test pushing pulling chunks over multiple requests", "[state]") {
        // Set up a chunk of state
        size_t chunkSize = 1024;
        size_t valueSize = 10 * chunkSize + 123;
        std::vector<uint8_t> values(valueSize, 1);
        DummyStateServer server;
        setUpDummyServer(server, values);

        // Two chunk pulls, one push partial
        server.start();

        // Set a chunk in the remote value
        size_t offsetA = 3 * chunkSize + 5;
        std::vector<uint8_t> segA = {4, 4};
        std::shared_ptr<state::StateKeyValue> remoteKv = server.getRemoteKv();
        remoteKv->setChunk(offsetA, segA.data(), segA.size());

        // Set a chunk at the end of the remote value
        std::vector<uint8_t> segB = {5, 5, 5, 5, 5};
        int offsetB = valueSize - 1 - segB.size();
        remoteKv->setChunk(offsetB, segB.data(), segB.size());

        // Get only these chunks locally
        std::shared_ptr<state::StateKeyValue> localKv = server.getLocalKv();
        std::vector<uint8_t> actualSegA(segA.size(), 0);
        localKv->getChunk(offsetA, actualSegA.data(), segA.size());
        REQUIRE(actualSegA == segA);

        std::vector<uint8_t> actualSegB(segB.size(), 0);
        localKv->getChunk(offsetB, actualSegB.data(), segB.size());
        REQUIRE(actualSegB == segB);

        // Now modify a different chunk locally
        size_t offsetC = 2 * chunkSize + 2;
        std::vector<uint8_t> segC = {0, 1, 2, 3, 4};
        localKv->setChunk(offsetC, segC.data(), segC.size());

        // Modify a chunk right at the end
        std::vector<uint8_t> segD = {3, 3, 3};
        int offsetD = valueSize - 1 - segD.size();
        localKv->setChunk(offsetD, segD.data(), segD.size());

        // Push the changes
        localKv->pushPartial();

        // Check the chunks in the remote value
        std::vector<uint8_t> actualAfterPush = server.getRemoteKvValue();
        std::vector<uint8_t> actualSegC(actualAfterPush.begin() + offsetC,
                                        actualAfterPush.begin() + offsetC + segC.size());
        std::vector<uint8_t> actualSegD(actualAfterPush.begin() + offsetD,
                                        actualAfterPush.begin() + offsetD + segD.size());

        REQUIRE(actualSegC == segC);
        REQUIRE(actualSegD == segD);

        DummyStateServer::stop();
        server.wait();
    }

    TEST_CASE("Test pulling disjoint chunks of the same value which share pages", "[state]") {
        // Set up state
        size_t valueSize = 20 * util::HOST_PAGE_SIZE + 123;
        std::vector<uint8_t> values(valueSize, 1);
        DummyStateServer server;
        setUpDummyServer(server, values);

        // Expect two chunk pulls
        server.start();

        // Set up two chunks both from the same page of memory but not overlapping
        long offsetA = 2 * util::HOST_PAGE_SIZE + 10;
        long offsetB = 2 * util::HOST_PAGE_SIZE + 20;
        long lenA = 5;
        long lenB = 10;

        std::vector<uint8_t> actualA(lenA, 0);
        std::vector<uint8_t> expectedA(lenA, 1);
        std::vector<uint8_t> actualB(lenA, 0);
        std::vector<uint8_t> expectedB(lenA, 1);

        const std::shared_ptr<state::StateKeyValue> &localKv = server.getLocalKv();
        localKv->getChunk(offsetA, actualA.data(), lenA);
        localKv->getChunk(offsetB, actualB.data(), lenB);

        // Check the local KV has been initialised to the right size
        REQUIRE(localKv->getSharedMemorySize() == 21 * util::HOST_PAGE_SIZE);

        // Check both chunks are as expected
        REQUIRE(actualA == expectedA);
        REQUIRE(actualB == expectedB);

        DummyStateServer::stop();
        server.wait();
    }
}