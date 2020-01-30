#include <catch/catch.hpp>
#include "utils.h"

using namespace mpi;

namespace tests {

    TEST_CASE("Basic getters/setters", "[mpi]") {
        MpiContext c;
        REQUIRE(!c.getIsMpi());
        REQUIRE(c.getMpiWorldId() == -1);
        REQUIRE(c.getMpiRank() == -1);

        c.setIsMpi(true);
        c.setMpiWorldId(345);
        c.setMpiRank(123);

        REQUIRE(c.getIsMpi());
        REQUIRE(c.getMpiWorldId() == 345);
        REQUIRE(c.getMpiRank() == 123);
    }
}