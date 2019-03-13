#pragma once

#include <stdint.h>
#include <algorithm>

namespace memory {
    class MemorySnapshot {
    public:
        MemorySnapshot();

        void create(const char * name, uint8_t *mem, size_t memSize);

        void createCopy(uint8_t *mem, size_t memSize);

        void restore(uint8_t * target);

        void restoreCopy(uint8_t *target);

        void clear();

    private:
        size_t memSize;
        int memFd;
        uint8_t *memCopy;
    };
}
