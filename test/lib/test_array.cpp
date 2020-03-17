#include <catch/catch.hpp>

#include "utils.h"

#include <redis/Redis.h>
#include <state/State.h>

#include <faasm/array.h>
#include <emulator/emulator.h>

using namespace faasm;

namespace tests {
    TEST_CASE("Test array operations", "[counter]") {
        cleanSystem();

        state::State &state = state::getGlobalState();
        std::string user = getEmulatorUser();

        const char *key = "test_array";
        int arraySize = 10;
        size_t bytesSize = arraySize * sizeof(int);
        faasm::AsyncArray<int> arrayA(key, arraySize);
        arrayA.zero();

        // Check state and array point to the same thing
        auto stateKv = state.getKV(user, key, arraySize * sizeof(int));
        auto stateMem = reinterpret_cast<int *>(stateKv->get());
        REQUIRE(arrayA.data() == stateMem);

        // Check state is zeroed
        std::vector<int> actual(stateMem, stateMem + arraySize);
        std::vector<int> expected(arraySize, 0);
        REQUIRE(actual == expected);

        // Write values, check they're reflected in memory
        arrayA[5] = 55;
        arrayA[2] = 22;
        arrayA.push();

        REQUIRE(stateMem[5] == 55);
        REQUIRE(stateMem[2] == 22);
        REQUIRE(arrayA[5] == 55);
        REQUIRE(arrayA[2] == 22);

        // Create another instance and check same state is reflected
        faasm::AsyncArray<int> arrayB(key, arraySize);
        arrayB.pull();
        REQUIRE(arrayA.data() == arrayB.data());
        REQUIRE(arrayB[5] == 55);
        REQUIRE(arrayB[2] == 22);

        // Update the second instance
        arrayB[3] = 33;
        arrayB.push();

        // Clear the local state, reload and check first instance
        stateKv->clear();
        arrayA.pullLazy();
        REQUIRE(arrayA[5] == 55);
        REQUIRE(arrayA[2] == 22);
        REQUIRE(arrayA[3] == 33);
    }
}