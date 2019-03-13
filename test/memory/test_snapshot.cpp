#include <catch/catch.hpp>

#include "utils.h"

#include <memory/MemorySnapshot.h>

using namespace memory;

namespace tests {

    TEST_CASE("Test copy create/restore", "[memory]") {
        MemorySnapshot snap;

        std::vector<uint8_t> dataA = {0, 1, 2, 3, 4, 5};
        std::vector<uint8_t> dataB = {6, 6, 6, 6, 6, 6, 6, 6};

        snap.createCopy(dataA.data(), 3);

        snap.restoreCopy(dataB.data());

        std::vector<uint8_t> expected = {0, 1, 2, 6, 6, 6, 6, 6};
        REQUIRE(dataB == expected);
    }
}