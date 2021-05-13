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

    std::vector<uint32_t> sharedVars = { 22, 33, 44, 55, 66 };
    lvlA.setSharedVars(sharedVars.data(), sharedVars.size());
    REQUIRE(lvlA.getSharedVars() == sharedVars);

    // Make sure we serialise via the relevant protobuf object to test the round
    // trip
    std::vector<uint8_t> serialised = lvlA.serialise();
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("demo", "echo", 1);
    req->set_contextdata(serialised.data(), serialised.size());

    // Deliberately don't set right number of threads
    std::shared_ptr<Level> lvlB = levelFromBatchRequest(req);

    REQUIRE(lvlB->id == lvlA.id);
    REQUIRE(lvlB->activeLevels == lvlA.activeLevels);
    REQUIRE(lvlB->maxActiveLevels == lvlA.maxActiveLevels);
    REQUIRE(lvlB->depth == lvlA.depth);
    REQUIRE(lvlB->globalTidOffset == lvlA.globalTidOffset);
    REQUIRE(lvlB->numThreads == lvlA.numThreads);
    REQUIRE(lvlB->pushedThreads == lvlA.pushedThreads);
    REQUIRE(lvlB->wantedThreads == lvlA.wantedThreads);

    REQUIRE(lvlB->getSharedVars() == sharedVars);
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
    std::atomic<int> sharedInt = 0;
    std::atomic<int> sharedSum = 0;

    int nThreads = 5;
    Level lvlA(nThreads);

    std::vector<uint8_t> serialised = lvlA.serialise();
    std::shared_ptr<faabric::BatchExecuteRequest> req =
      faabric::util::batchExecFactory("demo", "echo", 1);
    req->set_contextdata(serialised.data(), serialised.size());

    // Spawn n-1 child threads to wait on barriers
    std::vector<std::thread> threads;
    for (int i = 1; i < nThreads; i++) {
        threads.emplace_back([nThreads, &req, &sharedInt, &sharedSum] {
            UNUSED(nThreads);
            UNUSED(sharedInt);

            std::shared_ptr<Level> lvlB = levelFromBatchRequest(req);

            assert(lvlB->numThreads == nThreads);

            // Barrier 1
            lvlB->waitOnBarrier();

            // Make visible changes
            assert(sharedInt == 99);
            sharedSum.fetch_add(1);

            // Barrier 2
            lvlB->waitOnBarrier();
        });
    }

    // Block for a while as the child threads wait on the first barrier
    usleep(1000 * 1000);

    // Set the shared int that the threads should wake up to see
    sharedInt = 99;

    // Finish barrier one
    lvlA.waitOnBarrier();

    // Sleep again
    usleep(1000 * 1000);

    // Finish barrier two and check threads have done their work
    lvlA.waitOnBarrier();
    REQUIRE(sharedSum == nThreads - 1);

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

    int nThreads = 3;
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

            // Call the master wait
            lvlB->masterWait(i);

            // Check this thread doesn't have to wait
            assert(sharedInt == 0);

            // Add to the shared sum
            sharedSum += 1;
        });
    }

    // Do the master wait from the master thread
    lvlA.masterWait(0);

    // Make a change that would be seen by threads if the wait didn't work
    sharedInt = 99;

    // Check the changes from the child threads
    REQUIRE(sharedSum == nThreads - 1);

    // Join all child threads
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}
}
