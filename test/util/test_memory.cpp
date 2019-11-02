#include <catch/catch.hpp>
#include <util/memory.h>

using namespace util;

namespace tests {

    TEST_CASE("Test rounding down offsets to page size", "[memory]") {
        REQUIRE(util::roundOffsetDownToPage(2 * util::HOST_PAGE_SIZE) == 2);
        REQUIRE(util::roundOffsetDownToPage(2 * util::HOST_PAGE_SIZE + 25) == 2);

        REQUIRE(util::roundOffsetDownToPage(0) == 0);
        REQUIRE(util::roundOffsetDownToPage(22) == 0);

        REQUIRE(util::roundOffsetDownToPage(867 * util::HOST_PAGE_SIZE) == 867);
        REQUIRE(util::roundOffsetDownToPage(867 * util::HOST_PAGE_SIZE - 1) == 866);
    }
}