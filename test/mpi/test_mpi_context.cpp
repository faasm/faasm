#include <catch/catch.hpp>
#include "utils.h"

using namespace mpi;

namespace tests {

    TEST_CASE("Check creation", "[mpi]") {
        MpiContext c;

        message::Message msg = util::messageFactory("mpi", "hellompi");

        int worldSize = 10;
        c.create(msg, 123, worldSize);

        REQUIRE(c.getIsMpi());
        REQUIRE(c.getMpiWorldId() == 123);
        REQUIRE(c.getMpiRank() == 0);
        REQUIRE(c.getWorldSize() == 10);

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
        MpiContext c;
        REQUIRE(!c.getIsMpi());
        REQUIRE(c.getMpiWorldId() == -1);
        REQUIRE(c.getMpiRank() == -1);

        message::Message msg = util::messageFactory("mpi", "hellompi");
        msg.set_ismpi(true);
        msg.set_mpiworldid(345);
        msg.set_mpirank(123);
        c.join(msg);

        REQUIRE(c.getIsMpi());
        REQUIRE(c.getMpiWorldId() == 345);
        REQUIRE(c.getMpiRank() == 123);
    }
}