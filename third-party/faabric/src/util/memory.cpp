#include "memory.h"

#include <cstdint>
#include <utility>


namespace util {
    bool isPageAligned(void *ptr) {
        return (((uintptr_t) (const void *) (ptr)) % (HOST_PAGE_SIZE) == 0);
    }

    size_t getRequiredHostPages(size_t nBytes) {
        // Rounding up
        size_t nHostPages = (nBytes + util::HOST_PAGE_SIZE - 1) / util::HOST_PAGE_SIZE;
        return nHostPages;
    }

    size_t getRequiredHostPagesRoundDown(size_t nBytes) {
        // Relying on integer division rounding down
        size_t nHostPages = nBytes / util::HOST_PAGE_SIZE;
        return nHostPages;
    }

    size_t alignOffsetDown(size_t offset) {
        size_t nHostPages = getRequiredHostPagesRoundDown(offset);
        return nHostPages * util::HOST_PAGE_SIZE;
    }

    AlignedChunk getPageAlignedChunk(long offset, long length) {
        // Calculate the page boundaries
        auto nPagesOffset = (long) util::getRequiredHostPagesRoundDown(offset);
        auto nPagesUpper = (long) util::getRequiredHostPages(offset + length);
        long nPagesLength = nPagesUpper - nPagesOffset;

        long nBytesLength = nPagesLength * util::HOST_PAGE_SIZE;

        long nBytesOffset = nPagesOffset * util::HOST_PAGE_SIZE;

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

