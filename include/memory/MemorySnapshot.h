#pragma once

#include <stdint.h>
#include <algorithm>

namespace memory {
    class MemorySnapshot {
    public:
        MemorySnapshot();

        void create(uint8_t *mem, size_t memSize);

        void createCopy(uint8_t *mem, size_t memSize);

        void restore(void * target);

        void restoreCopy(uint8_t *target);

        void clear();

    private:
        size_t memSize;

        uint8_t *memCopy;
    };
}
