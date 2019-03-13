#include "MemorySnapshot.h"


namespace memory {
    MemorySnapshot::MemorySnapshot() {
        memSize = 0;
        memCopy = nullptr;
    }

    void MemorySnapshot::create(uint8_t *mem, size_t memSizeIn) {
        memSize = memSizeIn;

    }

    void MemorySnapshot::createCopy(uint8_t *mem, size_t memSizeIn) {
        memSize = memSizeIn;
        memCopy = new uint8_t[memSize];
        std::copy(mem, mem + memSize, memCopy);
    }

    void MemorySnapshot::restore(void *ptr) {

    }

    void MemorySnapshot::restoreCopy(uint8_t *ptr) {
        std::copy(memCopy, memCopy + memSize, ptr);
    }

    void MemorySnapshot::clear() {
        memSize = 0;
        delete[] memCopy;
    }
}
