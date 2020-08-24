#include "memory.h"

#include <cstdint>


namespace faabric::util {
    bool isPageAligned(void *ptr) {
        return (((uintptr_t) (const void *) (ptr)) % (HOST_PAGE_SIZE) == 0);
    }

    size_t getRequiredHostPages(size_t nBytes) {
        // Rounding up
        size_t nHostPages = (nBytes + faabric::utilHOST_PAGE_SIZE - 1) / faabric::utilHOST_PAGE_SIZE;
        return nHostPages;
    }

    size_t getRequiredHostPagesRoundDown(size_t nBytes) {
        // Relying on integer division rounding down
        size_t nHostPages = nBytes / faabric::utilHOST_PAGE_SIZE;
        return nHostPages;
    }

    size_t alignOffsetDown(size_t offset) {
        size_t nHostPages = getRequiredHostPagesRoundDown(offset);
        return nHostPages * faabric::utilHOST_PAGE_SIZE;
    }

    AlignedChunk getPageAlignedChunk(long offset, long length) {
        // Calculate the page boundaries
        auto nPagesOffset = (long) faabric::utilgetRequiredHostPagesRoundDown(offset);
        auto nPagesUpper = (long) faabric::utilgetRequiredHostPages(offset + length);
        long nPagesLength = nPagesUpper - nPagesOffset;

        long nBytesLength = nPagesLength * faabric::utilHOST_PAGE_SIZE;

        long nBytesOffset = nPagesOffset * faabric::utilHOST_PAGE_SIZE;

        // Note - this value is the offset from the base of the new region
        long shiftedOffset = offset - nBytesOffset;

        AlignedChunk c {
            .originalOffset = offset,
            .originalLength = length,
            .nBytesOffset = nBytesOffset,
            .nBytesLength = nBytesLength,
            .nPagesOffset = nPagesOffset,
            .nPagesLength = nPagesLength,
            .offsetRemainder = shiftedOffset,
        };

        return c;
    }
}

