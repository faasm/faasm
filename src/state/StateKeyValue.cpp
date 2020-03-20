#include "StateKeyValue.h"

#include <util/config.h>
#include <util/memory.h>
#include <util/locks.h>
#include <util/logging.h>
#include <util/timing.h>

#include <sys/mman.h>
#include <util/macros.h>

using namespace util;

namespace state {
    /**
     * Key/value
     */
    StateKeyValue::StateKeyValue(const std::string &keyIn, size_t sizeIn) : key(keyIn),
                                                                            backend(getBackend()),
                                                                            valueSize(sizeIn) {

        // Work out size of required shared memory
        size_t nHostPages = getRequiredHostPages(valueSize);
        sharedMemSize = nHostPages * HOST_PAGE_SIZE;
        sharedMemory = nullptr;

        isDirty = false;
        _fullyAllocated = false;

        // Set up flags
        dirtyMask = new uint8_t[sharedMemSize];

        zeroDirtyMask();

        allocatedMask = new uint8_t[sharedMemSize];

        zeroAllocatedMask();
    }

    void StateKeyValue::pull() {
        const std::shared_ptr<spdlog::logger> &logger = getLogger();
        logger->debug("Pulling state for {}", key);
        pullImpl(false);
    }

    bool StateKeyValue::isSegmentAllocated(long offset, size_t length) {
        // TODO - more efficient way of checking this
        auto allocatedMaskBytes = BYTES(allocatedMask);
        for (size_t i = 0; i < length; i++) {
            if (allocatedMaskBytes[offset + i] == 0) {
                return false;
            }
        }
        return true;
    }

    void StateKeyValue::pullImpl(bool onlyIfEmpty) {
        // Drop out if we already have the data and we don't care about updating
        {
            SharedLock lock(valueMutex);
            if (onlyIfEmpty && _fullyAllocated) {
                return;
            }
        }

        // Unique lock on the whole value
        FullLock lock(valueMutex);
        PROF_START(statePull)

        if (onlyIfEmpty && _fullyAllocated) {
            return;
        }

        // Initialise storage if necessary
        if (!_fullyAllocated) {
            initialiseStorage(true);
        }

        const std::shared_ptr<spdlog::logger> &logger = getLogger();

        // Read from the remote
        logger->debug("Pulling remote value for {}", key);
        auto memoryBytes = static_cast<uint8_t *>(sharedMemory);
        backend.get(key, memoryBytes, valueSize);

        PROF_END(statePull)
    }

    void StateKeyValue::pullSegmentImpl(bool onlyIfEmpty, long offset, size_t length) {
        // Drop out if we already have the data and we don't care about updating
        {
            SharedLock lock(valueMutex);
            if (onlyIfEmpty && (_fullyAllocated || isSegmentAllocated(offset, length))) {
                return;
            }
        }

        // Unique lock
        FullLock lock(valueMutex);
        PROF_START(stateSegmentPull)

        // Check condition again
        bool segmentAllocated = isSegmentAllocated(offset, length);
        if (onlyIfEmpty && (_fullyAllocated || segmentAllocated)) {
            return;
        }

        // Initialise the storage if empty
        if (!segmentAllocated) {
            allocateSegment(offset, length);
        }

        const std::shared_ptr<spdlog::logger> &logger = getLogger();

        // Note - redis ranges are inclusive, so we need to knock one off
        size_t rangeEnd = offset + length - 1;

        // Read from the remote
        logger->debug("Pulling remote segment ({}-{}) for {}", offset, offset + length, key);
        auto memoryBytes = static_cast<uint8_t *>(sharedMemory);
        backend.getRange(key, memoryBytes + offset, length, offset, rangeEnd);

        PROF_END(stateSegmentPull)
    }

    long StateKeyValue::waitOnRemoteLock() {
        PROF_START(remoteLock)
        const std::shared_ptr<spdlog::logger> &logger = getLogger();

        long remoteLockId = backend.acquireLock(key, remoteLockTimeout);
        unsigned int retryCount = 0;
        while (remoteLockId <= 0) {
            logger->debug("Waiting on remote lock for {} (loop {})", key, retryCount);

            if (retryCount >= remoteLockMaxRetries) {
                logger->error("Timed out waiting for lock on {}", key);
                break;
            }

            // Sleep for 1ms
            usleep(1000);

            remoteLockId = backend.acquireLock(key, remoteLockTimeout);
            retryCount++;
        }

        PROF_END(remoteLock)
        return remoteLockId;
    }

    void StateKeyValue::get(uint8_t *buffer) {
        pullImpl(true);

        SharedLock lock(valueMutex);

        auto bytePtr = static_cast<uint8_t *>(sharedMemory);
        std::copy(bytePtr, bytePtr + valueSize, buffer);
    }

    uint8_t *StateKeyValue::get() {
        pullImpl(true);

        SharedLock lock(valueMutex);

        return static_cast<uint8_t *>(sharedMemory);
    }

    void StateKeyValue::getSegment(long offset, uint8_t *buffer, size_t length) {
        pullSegmentImpl(true, offset, length);

        SharedLock lock(valueMutex);

        // Return just the required segment
        if ((offset + length) > valueSize) {
            const std::shared_ptr<spdlog::logger> &logger = getLogger();

            logger->error("Out of bounds read at {} on {} with length {}", offset + length, key, valueSize);
            throw std::runtime_error("Out of bounds read");
        }

        auto bytePtr = static_cast<uint8_t *>(sharedMemory);
        std::copy(bytePtr + offset, bytePtr + offset + length, buffer);
    }

    uint8_t *StateKeyValue::getSegment(long offset, long len) {
        pullSegmentImpl(true, offset, len);

        SharedLock lock(valueMutex);

        uint8_t *segmentPtr = static_cast<uint8_t *>(sharedMemory) + offset;
        return segmentPtr;
    }

    void StateKeyValue::set(const uint8_t *buffer) {
        // Unique lock for setting the whole value
        FullLock lock(valueMutex);

        if (sharedMemory == nullptr) {
            initialiseStorage(true);
        }

        // Copy data into shared region
        std::copy(buffer, buffer + valueSize, static_cast<uint8_t *>(sharedMemory));
        isDirty = true;
    }

    void StateKeyValue::setSegment(long offset, const uint8_t *buffer, size_t length) {
        const std::shared_ptr<spdlog::logger> &logger = getLogger();

        // Check we're in bounds
        size_t end = offset + length;
        if (end > valueSize) {
            logger->error("Trying to write segment finishing at {} (value length {})", end, valueSize);
            throw std::runtime_error("Attempting to set segment out of bounds");
        }

        FullLock lock(valueMutex);

        // If necessary, allocate the memory
        if (!isSegmentAllocated(offset, length)) {
            allocateSegment(offset, length);
        }

        // Check size
        if (offset + length > valueSize) {
            logger->error("Segment length {} at offset {} too big for size {}", length, offset, valueSize);
            throw std::runtime_error("Setting state segment too big for container");
        }

        // Do the copy
        auto bytePtr = static_cast<uint8_t *>(sharedMemory);
        std::copy(buffer, buffer + length, bytePtr + offset);

        flagSegmentDirty(offset, length);
    }

    void StateKeyValue::flagDirty() {
        isDirty = true;
    }

    void StateKeyValue::zeroDirtyMask() {
        memset(dirtyMask, 0, valueSize);
    }

    void StateKeyValue::zeroAllocatedMask() {
        memset(allocatedMask, 0, valueSize);
    }

    void StateKeyValue::zeroValue() {
        util::FullLock lock(valueMutex);

        memset(sharedMemory, 0, valueSize);
    }

    void StateKeyValue::flagSegmentDirty(long offset, long len) {
        isDirty |= true;
        memset(((uint8_t *) dirtyMask) + offset, 0b11111111, len);
    }

    void StateKeyValue::flagSegmentAllocated(long offset, long len) {
        memset(((uint8_t *) allocatedMask) + offset, 0b11111111, len);
    }

    std::string StateKeyValue::getSegmentKey(long offset, long length) {
        util::SharedLock lock(valueMutex);

        std::string regionKey = key + "_" + std::to_string(offset) + "_" + std::to_string(length);

        return regionKey;
    }

    void StateKeyValue::clear() {
        FullLock lock(valueMutex);

        const std::shared_ptr<spdlog::logger> &logger = getLogger();
        logger->debug("Clearing value {}", key);

        // Set flag to say this is effectively new again
        _fullyAllocated = false;
        zeroDirtyMask();
        zeroAllocatedMask();
    }

    void StateKeyValue::deleteGlobal() {
        // Clear locally
        clear();

        // Delete remote
        FullLock lock(valueMutex);
        backend.del(key);
    }

    size_t StateKeyValue::size() {
        return valueSize;
    }

    void StateKeyValue::mapSharedMemory(void *destination, long pagesOffset, long nPages) {
        PROF_START(mapSharedMem)
        const std::shared_ptr<spdlog::logger> &logger = getLogger();

        if (!isPageAligned(destination)) {
            logger->error("Non-aligned destination for shared mapping of {}", key);
            throw std::runtime_error("Mapping misaligned shared memory");
        }

        // Check everything lines up first of all
        size_t offset = pagesOffset * util::HOST_PAGE_SIZE;
        size_t length = nPages * util::HOST_PAGE_SIZE;

        // Pull the value
        if (pagesOffset > 0 || length < valueSize) {
            pullSegmentImpl(true, offset, length);
        } else {
            pullImpl(true);
        }

        FullLock lock(valueMutex);

        // Remap the relevant pages of shared memory onto the new region
        auto sharedMemoryBytes = BYTES(sharedMemory);
        void *result = mremap(sharedMemoryBytes + offset, 0, length, MREMAP_FIXED | MREMAP_MAYMOVE, destination);
        if (result == MAP_FAILED) {
            logger->error("Failed mapping for {} at {} with size {}. errno: {} ({})",
                          key, offset, length, errno, strerror(errno));

            throw std::runtime_error("Failed mapping shared memory");
        }

        if (destination != result) {
            logger->error("New mapped addr doesn't match required {} != {}", destination, result);
            throw std::runtime_error("Misaligned shared memory mapping");
        }

        PROF_END(mapSharedMem)
    }

    void StateKeyValue::unmapSharedMemory(void *mappedAddr) {
        FullLock lock(valueMutex);
        const std::shared_ptr<spdlog::logger> &logger = getLogger();

        if (!isPageAligned(mappedAddr)) {
            logger->error("Attempting to unmap non-page-aligned memory at {} for {}", mappedAddr, key);
            throw std::runtime_error("Unmapping misaligned shared memory");
        }

        // Unmap the current memory so it can be reused
        int result = munmap(mappedAddr, sharedMemSize);
        if (result == -1) {
            logger->error("Failed to unmap shared memory at {} with size {}. errno: {}", mappedAddr, sharedMemSize,
                          errno);

            throw std::runtime_error("Failed unmapping shared memory");
        }
    }

    void StateKeyValue::allocateSegment(long offset, size_t length) {
        initialiseStorage(false);

        const std::shared_ptr<spdlog::logger> &logger = getLogger();

        // Work out the aligned region to allocate
        long alignedOffset = util::alignOffsetDown(offset);
        long alignedLength = (offset - alignedOffset) + (long) length;

        auto memBytes = BYTES(sharedMemory);
        int res = mprotect(memBytes + alignedOffset, alignedLength, PROT_WRITE);
        if (res != 0) {
            logger->debug("Mmapping of storage size {} failed. errno: {}", sharedMemSize, errno);

            throw std::runtime_error("Failed mapping memory for KV");
        }

        // Flag the segment as allocated
        flagSegmentAllocated(alignedOffset, alignedLength);
    }

    void StateKeyValue::initialiseStorage(bool allocate) {
        PROF_START(initialiseStorage)

        // Don't need to initialise twice
        if (sharedMemory != nullptr) {
            return;
        }

        const std::shared_ptr<spdlog::logger> &logger = getLogger();

        if (sharedMemSize == 0) {
            throw StateKeyValueException("Initialising storage without a size for " + key);
        }

        // Note - only make memory writable if we want to allocate it fully up-front
        int prot = PROT_NONE;
        if (allocate) {
            prot = PROT_WRITE;
        }

        // Create shared memory region for the value
        sharedMemory = mmap(nullptr, sharedMemSize, prot, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if (sharedMemory == MAP_FAILED) {
            logger->debug("Mmapping of storage size {} failed. errno: {}", sharedMemSize, errno);

            throw std::runtime_error("Failed mapping memory for KV");
        }

        size_t nPages = sharedMemSize / HOST_PAGE_SIZE;
        if (allocate) {
            logger->debug("Allocated {} pages of shared storage for {}", nPages, key);
        } else {
            logger->debug("Reserved {} pages of shared storage for {}", nPages, key);
        }

        // Flag that allocation has happened
        if (allocate) {
            _fullyAllocated = true;
        }

        PROF_END(initialiseStorage)
    }

    void StateKeyValue::pushFull() {
        // Ignore if not dirty
        if (!isDirty) {
            return;
        }

        PROF_START(pushFull)

        // Get full lock for complete push
        FullLock fullLock(valueMutex);

        // Double check condition
        if (!isDirty) {
            return;
        }

        const std::shared_ptr<spdlog::logger> &logger = getLogger();
        logger->debug("Pushing whole value for {}", key);

        backend.set(key, static_cast<uint8_t *>(sharedMemory), valueSize);

        // Remove any dirty flags
        isDirty = false;
        zeroDirtyMask();

        PROF_END(pushFull)
    }

    void StateKeyValue::pushPartialMask(const std::shared_ptr<StateKeyValue> &maskKv) {
        if (maskKv->valueSize != valueSize) {
            std::string msg =
                    "Different sizes: mask=" + std::to_string(maskKv->valueSize) +
                    " and value=" + std::to_string(valueSize);

            throw StateKeyValueException(msg);
        }

        uint8_t *maskPtr = maskKv->get();
        doPushPartial(maskPtr);
    }

    void StateKeyValue::pushPartial() {
        auto dirtyMaskBytes = BYTES(dirtyMask);
        doPushPartial(dirtyMaskBytes);
    }

    void StateKeyValue::doPushPartial(const uint8_t *dirtyMaskBytes) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Ignore if not dirty
        if (!isDirty) {
            return;
        }

        PROF_START(pushPartial)

        // We need a full lock while doing this, mainly to ensure no other threads start
        // the same process
        FullLock lock(valueMutex);

        // Double check condition
        if (!isDirty) {
            logger->debug("Ignoring partial push on {}", key);
            return;
        }
        // Iterate through and pipeline the dirty segments
        auto sharedMemoryBytes = BYTES(sharedMemory);
        long updateCount = 0;
        long startIdx = 0;
        bool isOn = false;

        for (size_t i = 0; i < valueSize; i++) {
            if (dirtyMaskBytes[i] == 0) {
                // If we encounter an "off" mask and we're "on", switch off and write the segment
                if (isOn) {
                    isOn = false;

                    // Pipeline the change
                    unsigned long length = i - startIdx;
                    backend.setRangePipeline(key, startIdx, sharedMemoryBytes + startIdx, length);
                    updateCount++;
                }
            } else {
                if (!isOn) {
                    isOn = true;
                    startIdx = i;
                }
            }
        }

        // Write a final chunk
        if (isOn) {
            unsigned long length = valueSize - startIdx;
            backend.setRangePipeline(key, startIdx, sharedMemoryBytes + startIdx, length);
            updateCount++;
        }

        // Zero the mask now that we're finished with it
        memset((void *) dirtyMaskBytes, 0, valueSize);

        // Flush the pipeline
        logger->debug("Pipelined {} updates on {}", updateCount, key);
        backend.flushPipeline(updateCount);

        // Read the latest value
        if (_fullyAllocated) {
            logger->debug("Pulling from remote on partial push for {}", key);
            backend.get(key, sharedMemoryBytes, valueSize);
        }

        // Mark as no longer dirty
        isDirty = false;

        PROF_END(pushPartial)
    }


    void StateKeyValue::lockGlobal() {
        lastRemoteLockId = this->waitOnRemoteLock();
    }

    void StateKeyValue::unlockGlobal() {
        backend.releaseLock(key, lastRemoteLockId);
    }

    void StateKeyValue::lockRead() {
        valueMutex.lock_shared();
    }

    void StateKeyValue::unlockRead() {
        valueMutex.unlock_shared();
    }

    void StateKeyValue::lockWrite() {
        valueMutex.lock();
    }

    void StateKeyValue::unlockWrite() {
        valueMutex.unlock();
    }

}