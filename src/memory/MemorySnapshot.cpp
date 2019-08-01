#include "MemorySnapshot.h"

#include <sys/mman.h>
#include <util/locks.h>
#include <unistd.h>
#include <stdexcept>


namespace memory {
    MemorySnapshot::MemorySnapshot() {
        memFd = -1;
        memSize = 0;
    }

    bool MemorySnapshot::createIfNotExists(const char *name, uint8_t *mem, size_t memSizeIn) {
        if (memSize == 0) {
            util::UniqueLock lock(mx);
            if (memSize == 0) {
                this->doCreate(name, mem, memSizeIn);
                return true;
            }
        }

        return false;
    }

    void MemorySnapshot::create(const char *name, uint8_t *mem, size_t memSizeIn) {
        util::UniqueLock lock(mx);
        this->doCreate(name, mem, memSizeIn);
    }

    void MemorySnapshot::doCreate(const char *name, uint8_t *mem, size_t memSizeIn) {
        memSize = memSizeIn;

        // Create an in-memory fd
        memFd = memfd_create(name, 0);

        // Set size
        if (ftruncate(memFd, memSize) != 0) {
            throw std::runtime_error("Failed to truncate memory");
        }

        // Write memory into fd
        if (write(memFd, mem, memSize) < 0) {
            throw std::runtime_error("Failed to write snapshot to memory");
        }
    }

    void MemorySnapshot::restore(uint8_t *ptr) {
        // Map the pointer onto the fd
        mmap(ptr, memSize, PROT_WRITE, MAP_PRIVATE | MAP_FIXED, memFd, 0);
    }

    size_t MemorySnapshot::getSize() {
        util::UniqueLock lock(mx);
        return memSize;
    }

    void MemorySnapshot::clear() {
        util::UniqueLock lock(mx);
        memSize = 0;
    }
}
