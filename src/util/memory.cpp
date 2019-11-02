#include "memory.h"

#include <stdint.h>

namespace util {
    bool isPageAligned(void *ptr) {
        return (((uintptr_t) (const void *) (ptr)) % (HOST_PAGE_SIZE) == 0);
    }

    size_t getRequiredHostPages(size_t nBytes) {
        size_t nHostPages = (nBytes + util::HOST_PAGE_SIZE - 1) / util::HOST_PAGE_SIZE;

        return nHostPages;
    }

    size_t roundOffsetDownToPage(size_t offset) {
        // Relying on integer division rounding down
        size_t nHostPages = offset / util::HOST_PAGE_SIZE;
        return nHostPages;
    }
}

