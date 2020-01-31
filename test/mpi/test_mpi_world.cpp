#include <catch/catch.hpp>
#include <mpi/MpiWorldRegistry.h>
#include <util/random.h>
#include "utils.h"

using namespace mpi;

namespace tests {

    static int worldId = 123;
    static int worldSize = 10;
    static const char* user = "mpi";
    static const char* func = "hellompi";

    TEST_CASE("Test world creation", "[mpi]") {
        cleanSystem();

        // Create the world
        const message::Message &msg = util::messageFactory(user, func);
        mpi::MpiWorld world;
        world.create(msg, worldId, worldSize);

        REQUIRE(world.getSize() == worldSize);
        REQUIRE(world.getId() == worldId);
        REQUIRE(world.getUser() == user);
        REQUIRE(world.getFunction() == func);

        // Check that chained function calls are made as expected
        scheduler::Scheduler &sch = scheduler::getScheduler();
        std::set<int> ranksFound;
        for(int i = 0; i < worldSize - 1; i++) {
            message::Message actualCall = sch.getFunctionQueue(msg)->dequeue();
            REQUIRE(actualCall.user() == user);
            REQUIRE(actualCall.function() == func);
            REQUIRE(actualCall.ismpi());
            REQUIRE(actualCall.mpiworldid() == worldId);
            REQUIRE(actualCall.mpirank() == i + 1);
        }
        
        // Check that this node is registered as the master
        const std::string actualNodeId = world.getNodeForRank(0);
        REQUIRE(actualNodeId == util::getNodeId());
    }

    TEST_CASE("Test world loading from state", "[mpi]") {
        cleanSystem();

        // Create a world
        const message::Message &msg = util::messageFactory(user, func);
        mpi::MpiWorld worldA;
        worldA.create(msg, worldId, worldSize);

        // Create another copy from state
        mpi::MpiWorld worldB;
        worldB.initialiseFromState(msg, worldId);

        REQUIRE(worldB.getSize() == worldSize);
        REQUIRE(worldB.getId() == worldId);
        REQUIRE(worldB.getUser() == user);
        REQUIRE(worldB.getFunction() == func);
    }

    TEST_CASE("Test registering a rank", "[mpi]") {
        cleanSystem();

        std::string nodeIdA = util::randomString(NODE_ID_LEN);
        std::string nodeIdB = util::randomString(NODE_ID_LEN);

        // Create a world
        const message::Message &msg = util::messageFactory(user, func);
        mpi::MpiWorld worldA;
        worldA.overrideNodeId(nodeIdA);
        worldA.create(msg, worldId, worldSize);

        // Register a rank to this node and check
        int rankA = 5;
        worldA.registerRank(5);
        const std::string actualNodeId = worldA.getNodeForRank(0);
        REQUIRE(actualNodeId == nodeIdA);
        
        // Create a new instance of the world with a new node ID
        mpi::MpiWorld worldB;
        worldB.overrideNodeId(nodeIdB);
        worldB.initialiseFromState(msg, worldId);

        int rankB = 4;
        worldB.registerRank(4);

        // Now check both world instances report the same mappings
        REQUIRE(worldA.getNodeForRank(rankA) == nodeIdA);
        REQUIRE(worldA.getNodeForRank(rankB) == nodeIdB);
        REQUIRE(worldB.getNodeForRank(rankA) == nodeIdA);
        REQUIRE(worldB.getNodeForRank(rankB) == nodeIdB);
    }
}