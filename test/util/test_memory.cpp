#include <catch/catch.hpp>
#include <util/memory.h>

using namespace util;

namespace tests {

    TEST_CASE("Test rounding down offsets to page size", "[memory]") {
        REQUIRE(util::alignOffsetDown(2 * util::HOST_PAGE_SIZE) == 2 * util::HOST_PAGE_SIZE);
        REQUIRE(util::alignOffsetDown(2 * util::HOST_PAGE_SIZE + 25) == 2 * util::HOST_PAGE_SIZE);

        REQUIRE(util::alignOffsetDown(0) == 0);
        REQUIRE(util::alignOffsetDown(22) == 0);

        REQUIRE(util::alignOffsetDown(867 * util::HOST_PAGE_SIZE) == 867 * util::HOST_PAGE_SIZE);
        REQUIRE(util::alignOffsetDown(867 * util::HOST_PAGE_SIZE - 1) == 866 * util::HOST_PAGE_SIZE);
    }
}