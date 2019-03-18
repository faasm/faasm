#include "MemorySnapshot.h"

#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>

namespace memory {
    MemorySnapshot::MemorySnapshot() {
        memFd = -1;
        memSize = 0;
        memCopy = nullptr;
    }

    void MemorySnapshot::create(const char* name, uint8_t *mem, size_t memSizeIn) {
        memSize = memSizeIn;

        // Create an in-memory fd
        memFd = memfd_create(name, 0);

        // Set size and write memory into fd
        ftruncate(memFd, memSize);
        write(memFd, mem, memSize);
    }

    void MemorySnapshot::createCopy(uint8_t *mem, size_t memSizeIn) {
        memSize = memSizeIn;
        memCopy = new uint8_t[memSize];
        std::copy(mem, mem + memSize, memCopy);
    }

    void MemorySnapshot::restore(uint8_t *ptr) {
        // Map the pointer onto the fd
        mmap(ptr, memSize, PROT_WRITE, MAP_PRIVATE | MAP_FIXED, memFd, 0);
    }

    void MemorySnapshot::restoreCopy(uint8_t *ptr) {
        std::copy(memCopy, memCopy + memSize, ptr);
    }

    size_t MemorySnapshot::getSize() {
        return memSize;
    }

    void MemorySnapshot::clear() {
        memSize = 0;
        delete[] memCopy;
    }
}
