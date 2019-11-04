#pragma once

#include <unistd.h>

namespace util {
    static const long HOST_PAGE_SIZE = sysconf(_SC_PAGESIZE);

    bool isPageAligned(void *ptr);

    size_t getRequiredHostPages(size_t nBytes);

    size_t getRequiredHostPagesRoundDown(size_t nBytes);

    size_t alignOffsetDown(size_t offset);
}