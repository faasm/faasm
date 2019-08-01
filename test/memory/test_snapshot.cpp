#include <catch/catch.hpp>

#include "utils.h"

#include <memory/MemorySnapshotRegister.h>
#include <memory/MemorySnapshot.h>

#include <sys/mman.h>

using namespace memory;

namespace tests {
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

    TEST_CASE("Test create if not exists", "[memory]") {
        const char *name = "my_mem2";
        MemorySnapshotRegister &reg = getGlobalMemorySnapshotRegister();

        std::shared_ptr<MemorySnapshot> snap = reg.getSnapshot(name);

        std::vector<uint8_t> dataA = {0, 1, 2, 3};
        std::vector<uint8_t> dataB = {4, 3, 2};

        // First time should create
        bool createdA = snap->createIfNotExists(name, dataA.data(), dataA.size());
        REQUIRE(createdA);
        REQUIRE(snap->getSize() == dataA.size());

        // Second time should not
        bool createdB = snap->createIfNotExists(name, dataB.data(), dataB.size());
        REQUIRE(!createdB);
        REQUIRE(snap->getSize() == dataA.size());

        // Should create after clearing
        snap->clear();
        bool createdC = snap->createIfNotExists(name, dataB.data(), dataB.size());
        REQUIRE(createdC);
        REQUIRE(snap->getSize() == dataB.size());
    }

}