#include <catch/catch.hpp>

#include <faasm/state.h>

using namespace faasm;

namespace tests {
    TEST_CASE("Test masking doubles", "[state]") {
        // Mask of zeros to start with
        std::vector<uint8_t> byteMaskArray = {
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0
        };

        // Mask a double in the middle
        auto intArray = reinterpret_cast<unsigned int *>(byteMaskArray.data());
        maskDouble(intArray, 1);

        std::vector<uint8_t> expected = {
                0, 0, 0, 0, 0, 0, 0, 0,
                BIT_MASK_8, BIT_MASK_8, BIT_MASK_8, BIT_MASK_8, BIT_MASK_8, BIT_MASK_8, BIT_MASK_8, BIT_MASK_8,
                0, 0, 0, 0
        };
        REQUIRE(byteMaskArray == expected);
    }
}