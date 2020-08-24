#include <catch/catch.hpp>
#include <faabric/util/memory.h>
#include <sys/mman.h>
#include <unistd.h>
#include <faabric/util/macros.h>

using namespace faabric::util;

namespace tests {

    TEST_CASE("Test rounding down offsets to page size", "[memory]") {
        REQUIRE(faabric::utilalignOffsetDown(2 * faabric::utilHOST_PAGE_SIZE) == 2 * faabric::utilHOST_PAGE_SIZE);
        REQUIRE(faabric::utilalignOffsetDown(2 * faabric::utilHOST_PAGE_SIZE + 25) == 2 * faabric::utilHOST_PAGE_SIZE);

        REQUIRE(faabric::utilalignOffsetDown(0) == 0);
        REQUIRE(faabric::utilalignOffsetDown(22) == 0);

        REQUIRE(faabric::utilalignOffsetDown(867 * faabric::utilHOST_PAGE_SIZE) == 867 * faabric::utilHOST_PAGE_SIZE);
        REQUIRE(faabric::utilalignOffsetDown(867 * faabric::utilHOST_PAGE_SIZE - 1) == 866 * faabric::utilHOST_PAGE_SIZE);
    }

    TEST_CASE("Check CoW memory mapping", "[memory]") {
        size_t memSize = getpagesize();

        // Create a memory region with some data
        void *sharedVoid = mmap(nullptr, memSize, PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        int *sharedInt = reinterpret_cast<int *>(sharedVoid);
        for (int i = 0; i < 10; i++) {
            sharedInt[i] = i;
        }

        // Create an anonymous file, put in some data
        int fd = memfd_create("foobar", 0);
        ftruncate(fd, memSize);
        write(fd, sharedVoid, memSize);

        // Create two larger memory regions not yet writeable
        void *regionAVoid = mmap(nullptr, 3 * memSize, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        void *regionBVoid = mmap(nullptr, 3 * memSize, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        uint8_t *regionABytes = BYTES(regionAVoid);
        uint8_t *regionBBytes = BYTES(regionBVoid);

        // Allow writing to the middle segment of both
        mprotect(regionABytes + memSize, memSize, PROT_WRITE);
        mprotect(regionBBytes + memSize, memSize, PROT_WRITE);

        // Make one a CoW mapping onto the shared region
        mmap(regionABytes + memSize, memSize, PROT_WRITE, MAP_PRIVATE | MAP_FIXED, fd, 0);

        // Get pointers now writable/ mapped
        int *regionAInt = reinterpret_cast<int *>(regionABytes + memSize);
        int *regionBInt = reinterpret_cast<int *>(regionBBytes + memSize);

        // Check contents
        for (int i = 0; i < 10; i++) {
            REQUIRE(sharedInt[i] == i);
            REQUIRE(regionAInt[i] == i);
            REQUIRE(regionBInt[i] == 0);
        }

        // Now write to CoW, should not be reflected in original
        for (int i = 0; i < 10; i++) {
            regionAInt[i] = 2 * i;
        }

        // Check contents
        for (int i = 0; i < 10; i++) {
            REQUIRE(sharedInt[i] == i);
            REQUIRE(regionAInt[i] == 2 * i);
            REQUIRE(regionBInt[i] == 0);
        }
    }

    TEST_CASE("Check shared memory mapping", "[memory]") {
        int pageSize = getpagesize();
        size_t memSize = 4 * pageSize;

        // Create a shared memory region
        void *sharedVoid = mmap(nullptr, memSize, PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        // Create two other memory regions
        void *regionAVoid = mmap(nullptr, memSize, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        void *regionBVoid = mmap(nullptr, memSize, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        // Sanity check a write to the first region
        int *sharedInt = reinterpret_cast<int *>(sharedVoid);
        int *regionAInt = reinterpret_cast<int *>(regionAVoid);
        int *regionBInt = reinterpret_cast<int *>(regionBVoid);

        sharedInt[0] = 11;
        sharedInt[1] = 22;

        REQUIRE(sharedInt[0] == 11);
        REQUIRE(sharedInt[1] == 22);
        REQUIRE(regionAInt[0] == 0);
        REQUIRE(regionAInt[1] == 0);
        REQUIRE(regionBInt[0] == 0);
        REQUIRE(regionBInt[1] == 0);

        // Map the shared region onto both of the other regions
        mremap(sharedVoid, 0, memSize, MREMAP_FIXED | MREMAP_MAYMOVE, regionAVoid);
        mremap(sharedVoid, 0, memSize, MREMAP_FIXED | MREMAP_MAYMOVE, regionBVoid);

        // Check changes reflected
        REQUIRE(sharedInt[0] == 11);
        REQUIRE(sharedInt[1] == 22);
        REQUIRE(regionAInt[0] == 11);
        REQUIRE(regionAInt[1] == 22);
        REQUIRE(regionBInt[0] == 11);
        REQUIRE(regionBInt[1] == 22);

        // Check update in mapped region propagates
        regionAInt[1] = 33;
        REQUIRE(sharedInt[0] == 11);
        REQUIRE(sharedInt[1] == 33);
        REQUIRE(regionAInt[0] == 11);
        REQUIRE(regionAInt[1] == 33);
        REQUIRE(regionBInt[0] == 11);
        REQUIRE(regionBInt[1] == 33);

        // Check update in original propagates
        sharedInt[0] = 44;
        REQUIRE(sharedInt[0] == 44);
        REQUIRE(sharedInt[1] == 33);
        REQUIRE(regionAInt[0] == 44);
        REQUIRE(regionAInt[1] == 33);
        REQUIRE(regionBInt[0] == 44);
        REQUIRE(regionBInt[1] == 33);

        // Unmap and remap one of the mapped regions
        munmap(regionAVoid, memSize);
        mmap(regionAVoid, memSize, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        regionAInt[0] = 6;
        REQUIRE(sharedInt[0] == 44);
        REQUIRE(sharedInt[1] == 33);
        REQUIRE(regionAInt[0] == 6);
        REQUIRE(regionAInt[1] == 0);
        REQUIRE(regionBInt[0] == 44);
        REQUIRE(regionBInt[1] == 33);

        // Check updates still propagate between shared and remaining mapping
        sharedInt[0] = 55;
        regionBInt[1] = 66;
        REQUIRE(sharedInt[0] == 55);
        REQUIRE(sharedInt[1] == 66);
        REQUIRE(regionAInt[0] == 6);
        REQUIRE(regionAInt[1] == 0);
        REQUIRE(regionBInt[0] == 55);
        REQUIRE(regionBInt[1] == 66);
    }

    TEST_CASE("Test small aligned memory chunk", "[util]") {
        AlignedChunk actual = getPageAlignedChunk(0, 10);

        REQUIRE(actual.originalOffset == 0);
        REQUIRE(actual.originalLength == 10);
        REQUIRE(actual.nBytesOffset == 0);
        REQUIRE(actual.nBytesLength == faabric::utilHOST_PAGE_SIZE);
        REQUIRE(actual.nPagesOffset == 0);
        REQUIRE(actual.nPagesLength == 1);
        REQUIRE(actual.offsetRemainder == 0);
    }

    TEST_CASE("Test aligned memory chunks near page boundaries", "[util]") {
        long originalOffset = 2 * faabric::utilHOST_PAGE_SIZE - 1;
        long originalLength = 3;

        AlignedChunk actual = getPageAlignedChunk(originalOffset, originalLength);
        REQUIRE(actual.originalOffset == originalOffset);
        REQUIRE(actual.originalLength == originalLength);
        REQUIRE(actual.nPagesOffset == 1);
        REQUIRE(actual.nPagesLength == 2);
        REQUIRE(actual.nBytesOffset == 1 * faabric::utilHOST_PAGE_SIZE);
        REQUIRE(actual.nBytesLength == 2 * faabric::utilHOST_PAGE_SIZE);
        REQUIRE(actual.offsetRemainder == faabric::utilHOST_PAGE_SIZE - 1);
    }

    TEST_CASE("Test large offset memory chunk", "[util]") {
        long originalOffset = 2 * faabric::utilHOST_PAGE_SIZE + 33;
        long originalLength = 5 * faabric::utilHOST_PAGE_SIZE + 123;

        AlignedChunk actual = getPageAlignedChunk(originalOffset, originalLength);
        REQUIRE(actual.originalOffset == originalOffset);
        REQUIRE(actual.originalLength == originalLength);
        REQUIRE(actual.nPagesOffset == 2);
        REQUIRE(actual.nPagesLength == 6);
        REQUIRE(actual.nBytesOffset == 2 * faabric::utilHOST_PAGE_SIZE);
        REQUIRE(actual.nBytesLength == 6 * faabric::utilHOST_PAGE_SIZE);
        REQUIRE(actual.offsetRemainder == 33);
    }

    TEST_CASE("Test already aligned memory chunk", "[util]") {
        long originalOffset = 10 * faabric::utilHOST_PAGE_SIZE;
        long originalLength = 5 * faabric::utilHOST_PAGE_SIZE;

        AlignedChunk actual = getPageAlignedChunk(originalOffset, originalLength);
        REQUIRE(actual.originalOffset == originalOffset);
        REQUIRE(actual.originalLength == originalLength);
        REQUIRE(actual.nPagesOffset == 10);
        REQUIRE(actual.nPagesLength == 5);
        REQUIRE(actual.nBytesOffset == 10 * faabric::utilHOST_PAGE_SIZE);
        REQUIRE(actual.nBytesLength == 5 * faabric::utilHOST_PAGE_SIZE);
        REQUIRE(actual.offsetRemainder == 0);
    }
}