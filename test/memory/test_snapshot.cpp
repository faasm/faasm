#include <catch/catch.hpp>

#include "utils.h"

#include <memory/MemorySnapshotRegister.h>
#include <memory/MemorySnapshot.h>

#include <sys/mman.h>

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

    void _compareByteRegions(uint8_t *a, uint8_t *b, size_t len) {
        for (size_t i = 0; i < len; i++) {
            REQUIRE(a[i] == b[i]);
        }
    }

    void _checkRegionZeroed(uint8_t *a, size_t len) {
        for (size_t i = 0; i < len; i++) {
            REQUIRE(a[i] == 0);
        }
    }

    TEST_CASE("Test copy and restore in cow memory", "[memory]") {
        const char *name = "my_mem";
        MemorySnapshotRegister &reg = getGlobalMemorySnapshotRegister();

        std::shared_ptr<MemorySnapshot> snap = reg.getSnapshot(name);

        // Set up some memory
        size_t len = 20;
        uint8_t original[len];
        for (size_t i = 0; i < len; i++) {
            original[i] = 2;
        }

        // Create the snapshot
        snap->create(name, original, len);

        // Create two new regions
        auto regionA = reinterpret_cast<uint8_t *>(mmap(nullptr, len, PROT_WRITE,
                                                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));

        auto regionB = reinterpret_cast<uint8_t *>(mmap(nullptr, len, PROT_WRITE,
                                                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));

        _checkRegionZeroed(regionA, len);
        _checkRegionZeroed(regionB, len);

        // Restore both from the snapshot
        snap->restore(regionA);
        snap->restore(regionB);

        // Check they're both equal to original
        _compareByteRegions(regionA, original, 20);
        _compareByteRegions(regionB, original, 20);

        // Modify one region and check modifications don't propagate
        regionA[0]++;
        regionA[1]++;
        REQUIRE(regionA[0] != original[0]);
        REQUIRE(regionA[1] != original[1]);

        // Check the rest of the regions are still equal
        _compareByteRegions(regionA + 2, original + 2, len - 2);
        _compareByteRegions(regionB, original, len);
    }
}