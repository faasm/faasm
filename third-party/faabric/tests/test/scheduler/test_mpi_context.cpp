#include <catch/catch.hpp>
#include <scheduler/MpiContext.h>
#include <faasmpi/mpi.h>
#include <util/random.h>
#include "utils.h"

using namespace scheduler;

namespace tests {

    TEST_CASE("Check world creation", "[mpi]") {
        cleanSystem();

        message::Message msg = util::messageFactory("mpi", "hellompi");
        msg.set_mpiworldsize(10);

        MpiContext c;
        c.createWorld(msg);

        // Check a new world ID is created
        int worldId = c.getWorldId();
        REQUIRE(worldId > 0);

        // Check this context is set up
        REQUIRE(c.getIsMpi());
        REQUIRE(c.getRank() == 0);
        
        // Get the world and check it is set up
        MpiWorldRegistry &reg = scheduler::getMpiWorldRegistry();
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
        msg.set_mpiworldsize(10);

        // Try creating world
        MpiContext c;
        REQUIRE_THROWS(c.createWorld(msg));
    }

    TEST_CASE("Check default world size is set", "[mpi]") {
        cleanSystem();

        // Create message with non-zero rank
        message::Message msg = util::messageFactory("mpi", "hellompi");
        msg.set_mpirank(0);

        // Set a new world size
        util::SystemConfig &conf = util::getSystemConfig();
        int origSize = conf.defaultMpiWorldSize;
        int defaultWorldSize = 12;
        conf.defaultMpiWorldSize = defaultWorldSize;

        // Request different sizes
        int requestedWorldSize;
        SECTION("Under zero") {
            requestedWorldSize = -1;
        }
        SECTION("Zero") {
            requestedWorldSize = 0;
        }

        // Create the world
        MpiContext c;
        msg.set_mpiworldsize(requestedWorldSize);
        c.createWorld(msg);
        int worldId = c.getWorldId();

        // Check that the size is set to the default
        MpiWorldRegistry &reg = scheduler::getMpiWorldRegistry();
        MpiWorld &world = reg.getOrInitialiseWorld(msg, worldId);
        REQUIRE(world.getSize() == defaultWorldSize);

        // Reset config
        conf.defaultMpiWorldSize = origSize;
    }

    TEST_CASE("Check joining world", "[mpi]") {
        cleanSystem();

        const std::string expectedHost = util::getSystemConfig().endpointHost;
        
        message::Message msgA = util::messageFactory("mpi", "hellompi");
        int worldSize = 6;
        msgA.set_mpiworldsize(worldSize);

        // Use one context to create the world
        MpiContext cA;
        cA.createWorld(msgA);
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

        // Check rank is registered to this host
        MpiWorldRegistry &reg = scheduler::getMpiWorldRegistry();
        MpiWorld &world = reg.getOrInitialiseWorld(msgB, worldId);
        const std::string actualHost = world.getHostForRank(1);
    }
}