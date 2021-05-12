#include "utils.h"
#include <catch2/catch.hpp>

#include <faabric/proto/faabric.pb.h>
#include <faabric/util/bytes.h>
#include <faabric/util/func.h>

#include <threads/ThreadState.h>

using namespace threads;

namespace tests {

TEST_CASE("Check level serialisation and deserialisation", "[threads]")
{
    cleanSystem();

    Level lvlA(10);
    lvlA.id = 111;
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
}
