#include <catch/catch.hpp>
#include <mpi/MpiWorldRegistry.h>
#include "utils.h"

using namespace mpi;

namespace tests {

    TEST_CASE("Check world creation", "[mpi]") {
        cleanSystem();

        message::Message msg = util::messageFactory("mpi", "hellompi");
        int worldSize = 10;

        MpiContext c;
        c.createWorld(msg, worldSize);

        // Check a new world ID is created
        int worldId = c.getWorldId();
        REQUIRE(worldId > 0);

        // Check this context is set up
        REQUIRE(c.getIsMpi());
        REQUIRE(c.getRank() == 0);
        
        // Get the world and check it is set up
        MpiWorldRegistry &reg = mpi::getMpiWorldRegistry();
        MpiWorld &world = reg.getWorld(msg, worldId);
        REQUIRE(world.getWorldSize() == 10);
        REQUIRE(world.getUser() == "mpi");
        REQUIRE(world.getFunction() == "hellompi");

        // Check that chained function calls are made as expected
        scheduler::Scheduler &sch = scheduler::getScheduler();
        std::set<int> ranksFound;
        for(int i = 0; i < worldSize - 1; i++) {
            message::Message actualCall = sch.getFunctionQueue(msg)->dequeue();
            REQUIRE(actualCall.user() == "mpi");
            REQUIRE(actualCall.function() == "hellompi");
            REQUIRE(actualCall.ismpi());
            REQUIRE(actualCall.mpiworldid() == 123);
            REQUIRE(actualCall.mpirank() == i + 1);
        }
    }

    TEST_CASE("Check joining world", "[mpi]") {
        cleanSystem();

        message::Message msgA = util::messageFactory("mpi", "hellompi");
        int worldSize = 6;

        // Use one context to create the world
        MpiContext cA;
        cA.createWorld(msgA, worldSize);
        int worldId = cA.getWorldId();

        // Get one message formed by world creation
        scheduler::Scheduler &sch = scheduler::getScheduler();
        message::Message msgB = sch.getFunctionQueue(msgA)->dequeue();

        // Create another context and make sure it's not initialised
        MpiContext cB;
        REQUIRE(!cB.getIsMpi());
        REQUIRE(cB.getWorldId() == -1);
        REQUIRE(cB.getRank() == -1);

        // Join the world
        cB.joinWorld(msgB);

        REQUIRE(cB.getIsMpi());
        REQUIRE(cB.getWorldId() == worldId);
        REQUIRE(cB.getRank() == 1);

        // TODO - check rank is registered to this node
    }
}