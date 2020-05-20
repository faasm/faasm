#include "memory.h"

#include <stdint.h>

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
}

