#include <catch/catch.hpp>
#include "utils.h"

using namespace mpi;

namespace tests {

    TEST_CASE("Check creation", "[mpi]") {
        cleanSystem();

        message::Message msg = util::messageFactory("mpi", "hellompi");
        int worldSize = 10;

        MpiContext c;
        c.create(msg, 123, worldSize);

        REQUIRE(c.getIsMpi());
        REQUIRE(c.getMpiWorldId() == 123);
        REQUIRE(c.getMpiRank() == 0);
        REQUIRE(c.getWorldSize() == 10);
        REQUIRE(c.getUser() == "mpi");
        REQUIRE(c.getFunction() == "hellompi");

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

    TEST_CASE("Check joining", "[mpi]") {
        cleanSystem();

        message::Message msgA = util::messageFactory("mpi", "hellompi");
        int worldSize = 6;

        // Use one context to create the world
        MpiContext cA;
        cA.create(msgA, 123, worldSize);

        // Get one message
        scheduler::Scheduler &sch = scheduler::getScheduler();
        message::Message msgB = sch.getFunctionQueue(msgA)->dequeue();

        // Create another context
        MpiContext cB;
        REQUIRE(!cB.getIsMpi());
        REQUIRE(cB.getMpiWorldId() == -1);
        REQUIRE(cB.getMpiRank() == -1);

        // Join and check info propagated
        cB.join(msgB);

        REQUIRE(cB.getIsMpi());
        REQUIRE(cB.getMpiWorldId() == 345);
        REQUIRE(cB.getMpiRank() == 123);
        REQUIRE(cB.getWorldSize() == worldSize);
        REQUIRE(cB.getUser() == "mpi");
        REQUIRE(cB.getFunction() == "hellompi");
    }
}