#include <catch/catch.hpp>
#include <mpi/MpiContext.h>
#include <faasmpi/mpi.h>
#include <util/random.h>
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
        MpiWorld &world = reg.getOrInitialiseWorld(msg, worldId);
        REQUIRE(world.getId() == worldId);
        REQUIRE(world.getSize() == 10);
        REQUIRE(world.getUser() == "mpi");
        REQUIRE(world.getFunction() == "hellompi");
    }

    TEST_CASE("Check world cannot be created for non-zero rank", "[mpi]") {
        cleanSystem();

        // Create message with non-zero rank
        message::Message msg = util::messageFactory("mpi", "hellompi");
        msg.set_mpirank(2);

        // Try creating world
        MpiContext c;
        int worldSize = 10;
        REQUIRE_THROWS(c.createWorld(msg, worldSize));
    }

    TEST_CASE("Check joining world", "[mpi]") {
        cleanSystem();

        const std::string expectedNode = util::getNodeId();
        
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

        // Check rank is registered to this node
        MpiWorldRegistry &reg = mpi::getMpiWorldRegistry();
        MpiWorld &world = reg.getOrInitialiseWorld(msgB, worldId);
        const std::string actualNode = world.getNodeForRank(1);

        // Check message sent back to master on local queue
        const std::shared_ptr<InMemoryMpiQueue> &masterQueue = world.getLocalQueue(1, 0);
        MpiMessage *actualMessage = masterQueue->dequeue();
        REQUIRE(actualMessage->count == 0);
        REQUIRE(actualMessage->type == FAASMPI_INT);

        delete actualMessage;
    }

    TEST_CASE("Check awaiting world creation", "[mpi]") {
        cleanSystem();

        // Create a world from one context
        int worldSize = 4;
        message::Message msg = util::messageFactory("mpi", "hellompi");
        MpiContext c0;
        c0.createWorld(msg, worldSize);
        int worldId = c0.getWorldId();

        // Join this from several contexts
        MpiContext c1;
        msg.set_ismpi(true);
        msg.set_mpiworldid(worldId);
        msg.set_mpirank(1);
        c1.joinWorld(msg);

        MpiContext c2;
        msg.set_mpirank(2);
        c2.joinWorld(msg);

        MpiContext c3;
        msg.set_mpirank(3);
        c3.joinWorld(msg);

        MpiContext c4;
        msg.set_mpirank(4);
        c4.joinWorld(msg);

        // Check we've got the messages from the joinees
        MpiWorldRegistry &reg = mpi::getMpiWorldRegistry();
        MpiWorld &world = reg.getWorld(worldId);
        REQUIRE(world.getLocalQueueSize(1, 0) == 1);
        REQUIRE(world.getLocalQueueSize(2, 0) == 1);
        REQUIRE(world.getLocalQueueSize(3, 0) == 1);
        REQUIRE(world.getLocalQueueSize(4, 0) == 1);

        // Check that awaiting fails on non-zero rank context
        REQUIRE_THROWS(c1.awaitWorldCreation());
        REQUIRE_THROWS(c2.awaitWorldCreation());
        REQUIRE_THROWS(c3.awaitWorldCreation());
        REQUIRE_THROWS(c4.awaitWorldCreation());

        // Check that awaiting succeeds
        c0.awaitWorldCreation();
    }
}