#include <catch2/catch.hpp>

#include "utils.h"

#include <faabric/proto/faabric.pb.h>
#include <faabric/util/batch.h>
#include <faabric/util/bytes.h>
#include <faabric/util/macros.h>
#include <threads/ThreadState.h>

using namespace threads;

namespace tests {

TEST_CASE("Check level serialisation and deserialisation", "[threads]")
{
    Level lvlA(10);

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
    std::string buffer;
    REQUIRE(req->SerializeToString(&buffer));

    auto reqB = std::make_shared<faabric::BatchExecuteRequest>();
    reqB->ParseFromString(buffer);
    REQUIRE(reqB->contextdata().size() == expectedSize);

    // Deserialise the nested level object
    std::shared_ptr<Level> lvlB = levelFromBatchRequest(reqB);

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

    std::vector<uint32_t> sharedVarOffsets = { 22, 33, 44 };
    lvlA.setSharedVarOffsets(sharedVarOffsets.data(), sharedVarOffsets.size());

    Level lvlB(5);

    std::vector<uint8_t> serialisedA = lvlA.serialise();
    std::vector<uint8_t> serialisedB = lvlB.serialise();

    size_t sizeDiff = serialisedA.size() - serialisedB.size();
    REQUIRE(sizeDiff == sharedVarOffsets.size() * sizeof(uint32_t));
}
}
