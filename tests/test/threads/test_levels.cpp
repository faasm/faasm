#include "utils.h"
#include <catch2/catch.hpp>

#include <faabric/proto/faabric.pb.h>
#include <faabric/util/bytes.h>
#include <faabric/util/func.h>
#include <faabric/util/macros.h>

#include <threads/ThreadState.h>

using namespace threads;

namespace tests {

TEST_CASE("Check level serialisation and deserialisation", "[threads]")
{
    cleanSystem();

    Level lvlA(10);
    REQUIRE(lvlA.id != 0);

    lvlA.activeLevels = 222;
    lvlA.maxActiveLevels = 333;
    lvlA.depth = 444;
    lvlA.globalTidOffset = 555;
    lvlA.numThreads = 666;
    lvlA.pushedThreads = 777;
    lvlA.wantedThreads = 888;

    std::vector<uint32_t> sharedVarOffsets = { 22, 33, 44, 55, 66 };
    lvlA.setSharedVarOffsets(sharedVarOffsets.data(), sharedVarOffsets.size());
    REQUIRE(lvlA.getSharedVarOffsets() == sharedVarOffsets);

    // Serialise and check size
    std::vector<uint8_t> serialised = lvlA.serialise();
    size_t expectedSize =
      sizeof(Level) + (sharedVarOffsets.size() * sizeof(uint32_t));
    REQUIRE(serialised.size() == expectedSize);

    // Serialise via the relevant protobuf object to test the round trip
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("demo", "echo", 1);
    req->set_contextdata(serialised.data(), serialised.size());

    // Serialise and deserialise the protobuf object
    size_t bufferSize = req->ByteSizeLong();
    uint8_t buffer[bufferSize];
    req->SerializeToArray(buffer, bufferSize);

    auto reqB = std::make_shared<faabric::BatchExecuteRequest>();
    reqB->ParseFromArray(buffer, bufferSize);
    REQUIRE(reqB->contextdata().size() == expectedSize);

    // Deserialise the nested level object
    std::shared_ptr<Level> lvlB = levelFromBatchRequest(reqB);

    REQUIRE(lvlB->id == lvlA.id);
    REQUIRE(lvlB->activeLevels == lvlA.activeLevels);
    REQUIRE(lvlB->maxActiveLevels == lvlA.maxActiveLevels);
    REQUIRE(lvlB->depth == lvlA.depth);
    REQUIRE(lvlB->globalTidOffset == lvlA.globalTidOffset);
    REQUIRE(lvlB->numThreads == lvlA.numThreads);
    REQUIRE(lvlB->pushedThreads == lvlA.pushedThreads);
    REQUIRE(lvlB->wantedThreads == lvlA.wantedThreads);

    REQUIRE(lvlB->getSharedVarOffsets() == sharedVarOffsets);
}

TEST_CASE("Check level serialisation sizes", "[threads]")
{
    Level lvlA(10);
    REQUIRE(lvlA.id != 0);

    std::vector<uint32_t> sharedVarOffsets = { 22, 33, 44 };
    lvlA.setSharedVarOffsets(sharedVarOffsets.data(), sharedVarOffsets.size());

    Level lvlB(5);
    REQUIRE(lvlB.id != 0);

    std::vector<uint8_t> serialisedA = lvlA.serialise();
    std::vector<uint8_t> serialisedB = lvlB.serialise();

    size_t sizeDiff = serialisedA.size() - serialisedB.size();
    REQUIRE(sizeDiff == sharedVarOffsets.size() * sizeof(uint32_t));
}

TEST_CASE("Test level locking", "[threads]")
{
    cleanSystem();

    std::atomic<int> sharedInt = 0;

    // Set up the level and lock early
    Level lvlA(10);
    lvlA.lockCritical();

    uint32_t expectedId = lvlA.id;

    std::vector<uint8_t> serialised = lvlA.serialise();
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("demo", "echo", 1);
    req->set_contextdata(serialised.data(), serialised.size());

    std::thread tA([&req, &sharedInt, expectedId] {
        UNUSED(expectedId);

        std::shared_ptr<Level> lvlB = levelFromBatchRequest(req);
        assert(lvlB->id == expectedId);

        lvlB->lockCritical();

        assert(sharedInt == 99);
        sharedInt = 88;

        lvlB->unlockCritical();
    });

    // Main thread sleep for a while, make sure the other can't run and update
    // the counter
    usleep(1000 * 1000);

    REQUIRE(sharedInt == 0);
    sharedInt.store(99);

    lvlA.unlockCritical();

    if (tA.joinable()) {
        tA.join();
    }

    REQUIRE(sharedInt == 88);
}

TEST_CASE("Test level barrier", "[threads]")
{
    cleanSystem();

    int nThreads = 5;
    Level lvlA(nThreads);
    uint32_t levelId = lvlA.id;

    std::vector<uint8_t> serialised = lvlA.serialise();
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("demo", "echo", 1);
    req->set_contextdata(serialised.data(), serialised.size());

    // Spawn n-1 child threads to add to shared sums over several barriers so
    // that the main thread can check all threads have completed after each.
    // We want to do this as many times as possible to deliberately create
    // contention

    int nSums = 1000;
    std::vector<std::atomic<int>> sharedSums(nSums);
    std::vector<std::thread> threads;
    for (int i = 1; i < nThreads; i++) {
        threads.emplace_back([nThreads, levelId, &req, nSums, &sharedSums] {
            UNUSED(nThreads);
            UNUSED(levelId);

            std::shared_ptr<Level> lvlB = levelFromBatchRequest(req);
            assert(lvlB->numThreads == nThreads);
            assert(lvlB->id == levelId);

            for (int s = 0; s < nSums; s++) {
                sharedSums.at(s).fetch_add(s + 1);
                lvlB->waitOnBarrier();
            }
        });
    }

    for (int i = 0; i < nSums; i++) {
        lvlA.waitOnBarrier();
        REQUIRE(sharedSums.at(i).load() == (i + 1) * (nThreads - 1));
    }

    // Join all child threads
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}

TEST_CASE("Test nowait barrier", "[threads]")
{
    cleanSystem();
    std::atomic<int> sharedInt = 0;
    std::atomic<int> sharedSum = 0;

    int nThreads = 10;
    Level lvlA(nThreads);

    std::vector<uint8_t> serialised = lvlA.serialise();
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("demo", "echo", 1);
    req->set_contextdata(serialised.data(), serialised.size());

    // Set up the other threads to run through their nowaits
    std::vector<std::thread> threads;
    for (int i = 1; i < nThreads; i++) {
        threads.emplace_back([i, &req, &sharedInt, &sharedSum] {
            UNUSED(sharedInt);

            std::shared_ptr<Level> lvlB = levelFromBatchRequest(req);

            // Sleep for a while to make sure the master thread has hit the
            // barrier
            usleep(1000 * 1000);

            // Add to the shared sum
            sharedSum += 1;

            // Check the master thread's changes aren't yet visible as it should
            // be waiting
            assert(sharedInt == 0);

            // Notify the master we're done (should not have to wait)
            lvlB->masterWait(i);
        });
    }

    // Do the master wait from the master thread
    lvlA.masterWait(0);

    // Make a change that would be seen by threads if the wait didn't work
    sharedInt = 99;

    // Check other threads have finished
    REQUIRE(sharedSum == nThreads - 1);

    // Join all child threads
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}
}
