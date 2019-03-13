#include <catch/catch.hpp>

#include "utils.h"

#include <memory/MemorySnapshotRegister.h>
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

    TEST_CASE("Test snapshot registry", "[memory]") {
        std::vector<uint8_t> dataA = {0, 1, 2, 3, 4, 5};
        std::vector<uint8_t> dataB = {6, 6, 6, 6, 6, 6, 6, 6};
        std::vector<uint8_t> dataC = {1, 1, 1, 1, 1, 1};
        std::vector<uint8_t> dataD = {2, 2, 2, 2};

        std::string keyA = "snapA";
        std::string keyB = "snapB";

        // Get snapshots and copy memory
        MemorySnapshotRegister &reg = getGlobalMemorySnapshotRegister();
        std::shared_ptr<MemorySnapshot> snapA = reg.getSnapshot(keyA);
        std::shared_ptr<MemorySnapshot> snapB = reg.getSnapshot(keyB);

        snapA->createCopy(dataA.data(), 3);
        snapB->createCopy(dataB.data(), 4);

        // Retrieve the snaps again from the registry
        std::shared_ptr<MemorySnapshot> snapA2 = reg.getSnapshot(keyA);
        std::shared_ptr<MemorySnapshot> snapB2 = reg.getSnapshot(keyB);

        // Restore from these other refs
        snapA2->restoreCopy(dataC.data());
        snapB2->restoreCopy(dataD.data());

        std::vector<uint8_t> expectedC = {0, 1, 2, 1, 1, 1};
        std::vector<uint8_t> expectedD = {6, 6, 6, 6};
        REQUIRE(dataC == expectedC);
        REQUIRE(dataD == expectedD);
    }
}