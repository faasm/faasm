#pragma once

#include <stdint.h>
#include <algorithm>
#include <mutex>

namespace memory {
    class MemorySnapshot {
    public:
        MemorySnapshot();

        void create(const char * name, uint8_t *mem, size_t memSize);

        bool createIfNotExists(const char * name, uint8_t *mem, size_t memSize);

        void restore(uint8_t * target);

        void clear();

        size_t getSize();

    private:
        std::mutex mx;
        size_t memSize;
        int memFd;

        void doCreate(const char *name, uint8_t *mem, size_t memSizeIn);
    };
}
