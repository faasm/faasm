#include <catch/catch.hpp>
#include <util/memory.h>
#include <sys/mman.h>
#include <unistd.h>

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
        uint8_t *regionABytes = reinterpret_cast<uint8_t *>(regionAVoid);
        uint8_t *regionBBytes = reinterpret_cast<uint8_t *>(regionBVoid);

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
}