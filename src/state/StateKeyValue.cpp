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
    StateKeyValue::StateKeyValue(const std::string &userIn, const std::string &keyIn, size_t sizeIn) :
            user(userIn),
            key(keyIn),
            redis(redis::Redis::getState()),
            logger(util::getLogger()),
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
        logger->debug("Pulling state for {}/{}", user, key);
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

    void StateKeyValue::get(uint8_t *buffer) {
        pullImpl(true);

        LockRecursive lock(valueMutex);

        auto bytePtr = BYTES(sharedMemory);
        std::copy(bytePtr, bytePtr + valueSize, buffer);
    }

    uint8_t *StateKeyValue::get() {
        pullImpl(true);

        LockRecursive lock(valueMutex);

        return BYTES(sharedMemory);
    }

    void StateKeyValue::getSegment(long offset, uint8_t *buffer, size_t length) {
        pullSegmentImpl(true, offset, length);

        LockRecursive lock(valueMutex);

        // Return just the required segment
        if ((offset + length) > valueSize) {
            logger->error("Out of bounds read at {} on {}/{} with length {}",
                          offset + length, user, key, valueSize);
            throw std::runtime_error("Out of bounds read");
        }

        auto bytePtr = BYTES(sharedMemory);
        std::copy(bytePtr + offset, bytePtr + offset + length, buffer);
    }

    uint8_t *StateKeyValue::getSegment(long offset, long len) {
        pullSegmentImpl(true, offset, len);

        LockRecursive lock(valueMutex);

        uint8_t *segmentPtr = BYTES(sharedMemory) + offset;
        return segmentPtr;
    }

    void StateKeyValue::set(const uint8_t *buffer) {
        // Unique lock for setting the whole value
        LockRecursive lock(valueMutex);

        doSet(buffer);

        isDirty = true;
    }

    void StateKeyValue::doSet(const uint8_t *buffer) {
        if (sharedMemory == nullptr) {
            initialiseStorage(true);
        }

        // Copy data into shared region
        std::copy(buffer, buffer + valueSize, BYTES(sharedMemory));
    }

    void StateKeyValue::append(uint8_t *buffer, size_t length) {
        LockRecursive lock(valueMutex);

        appendToRemote(buffer, length);
    }

    void StateKeyValue::getAppended(uint8_t *buffer, size_t length, long nValues) {
        LockRecursive lock(valueMutex);

        pullAppendedFromRemote(buffer, length, nValues);
    }

    void StateKeyValue::setSegment(long offset, const uint8_t *buffer, size_t length) {
        LockRecursive lock(valueMutex);

        doSetSegment(offset, buffer, length);

        markDirtySegment(offset, length);
    }

    void StateKeyValue::doSetSegment(long offset, const uint8_t *buffer, size_t length) {
        // Check we're in bounds
        size_t end = offset + length;
        if (end > valueSize) {
            logger->error("Trying to write segment finishing at {} (value length {})", end, valueSize);
            throw std::runtime_error("Attempting to set segment out of bounds");
        }

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
        auto bytePtr = BYTES(sharedMemory);
        std::copy(buffer, buffer + length, bytePtr + offset);
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
        LockRecursive lock(valueMutex);

        memset(sharedMemory, 0, valueSize);
    }

    void StateKeyValue::flagSegmentDirty(long offset, long len) {
        // This is accessible publicly but also called internally when a
        // lock is already held, hence we need to split the locking and
        // marking of the segment.
        LockRecursive lock(valueMutex);
        markDirtySegment(offset, len);
    }

    void StateKeyValue::markDirtySegment(long offset, long len) {
        isDirty |= true;
        memset(((uint8_t *) dirtyMask) + offset, 0b11111111, len);
    }

    void StateKeyValue::markAllocatedSegment(long offset, long len) {
        memset(((uint8_t *) allocatedMask) + offset, 0b11111111, len);
    }

    void StateKeyValue::clear() {
        LockRecursive lock(valueMutex);

        logger->debug("Clearing value {}/{}", user, key);

        // Set flag to say this is effectively new again
        _fullyAllocated = false;
        zeroDirtyMask();
        zeroAllocatedMask();
    }

    size_t StateKeyValue::size() const {
        return valueSize;
    }

    void StateKeyValue::mapSharedMemory(void *destination, long pagesOffset, long nPages) {
        PROF_START(mapSharedMem)

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

        LockRecursive lock(valueMutex);

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
        LockRecursive lock(valueMutex);

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
        markAllocatedSegment(alignedOffset, alignedLength);
    }

    void StateKeyValue::initialiseStorage(bool allocate) {
        PROF_START(initialiseStorage)

        // Don't need to initialise twice
        if (sharedMemory != nullptr) {
            return;
        }

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

    void StateKeyValue::pushFull() {
        // Ignore if not dirty
        if (!isDirty) {
            return;
        }

        // Get full lock for complete push
        LockRecursive fullLock(valueMutex);

        // Double check condition
        if (!isDirty) {
            return;
        }

        pushToRemote();
    }

    void StateKeyValue::pullImpl(bool onlyIfEmpty) {
        // Drop out if we already have the data and we don't care about updating
        {
            LockRecursive lock(valueMutex);
            if (onlyIfEmpty && _fullyAllocated) {
                return;
            }
        }

        // Unique lock on the whole value
        LockRecursive lock(valueMutex);

        if (onlyIfEmpty && _fullyAllocated) {
            return;
        }

        // Initialise storage if necessary
        if (!_fullyAllocated) {
            initialiseStorage(true);
        }

        pullFromRemote();
    }

    void StateKeyValue::pullSegmentImpl(bool onlyIfEmpty, long offset, size_t length) {
        // Drop out if we already have the data and we don't care about updating
        {
            LockRecursive lock(valueMutex);
            if (onlyIfEmpty && (_fullyAllocated || isSegmentAllocated(offset, length))) {
                return;
            }
        }

        // Unique lock
        LockRecursive lock(valueMutex);

        // Check condition again
        bool segmentAllocated = isSegmentAllocated(offset, length);
        if (onlyIfEmpty && (_fullyAllocated || segmentAllocated)) {
            return;
        }

        // Initialise the storage if empty
        if (!segmentAllocated) {
            allocateSegment(offset, length);
        }

        pullRangeFromRemote(offset, length);
    }

    void StateKeyValue::doPushPartial(const uint8_t *dirtyMaskBytes) {
        // Ignore if not dirty
        if (!isDirty) {
            return;
        }

        // We need a full lock while doing this, mainly to ensure no other threads start
        // the same process
        LockRecursive lock(valueMutex);

        // Double check condition
        if (!isDirty) {
            logger->debug("Ignoring partial push on {}", key);
            return;
        }

        // Work out what's dirty
        const std::vector<StateChunk> &chunks = getDirtyChunks(dirtyMaskBytes);

        // Zero the mask now that we're finished with it
        memset((void *) dirtyMaskBytes, 0, valueSize);

        // Do the write
        pushPartialToRemote(chunks);

        // Mark as no longer dirty
        isDirty = false;
    }

    uint32_t StateKeyValue::waitOnRedisRemoteLock(const std::string &redisKey) {
        PROF_START(remoteLock)

        redis::Redis &redis = redis::Redis::getState();
        uint32_t remoteLockId = redis.acquireLock(redisKey, REMOTE_LOCK_TIMEOUT_SECS);
        unsigned int retryCount = 0;
        while (remoteLockId == 0) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->debug("Waiting on remote lock for {} (loop {})", redisKey, retryCount);

            if (retryCount >= REMOTE_LOCK_MAX_RETRIES) {
                logger->error("Timed out waiting for lock on {}", redisKey);
                break;
            }

            std::this_thread::sleep_for(std::chrono::microseconds (500 + rand() % 2500));

            remoteLockId = redis.acquireLock(redisKey, REMOTE_LOCK_TIMEOUT_SECS);
            retryCount++;
        }

        PROF_END(remoteLock)
        return remoteLockId;
    }


    void StateKeyValue::deleteGlobal() {
        // Clear locally
        clear();

        // Delete remote
        LockRecursive lock(valueMutex);
        deleteFromRemote();
    }

    void StateKeyValue::lockRead() {
        valueMutex.lock();
    }

    void StateKeyValue::unlockRead() {
        valueMutex.unlock();
    }

    void StateKeyValue::lockWrite() {
        valueMutex.lock();
    }

    void StateKeyValue::unlockWrite() {
        valueMutex.unlock();
    }

    std::vector<StateChunk> StateKeyValue::getDirtyChunks(const uint8_t *dirtyMaskBytes) {
        std::vector<StateChunk> chunks;

        auto sharedMemoryBytes = BYTES(sharedMemory);
        long startIdx = 0;
        bool isOn = false;

        // Iterate through looking for dirty chunks
        for (size_t i = 0; i < valueSize; i++) {
            if (dirtyMaskBytes[i] == 0) {
                // If we encounter an "off" mask and we're "on", switch off and write the segment
                if (isOn) {
                    isOn = false;

                    // Record the chunk
                    unsigned long length = i - startIdx;
                    chunks.emplace_back(startIdx, length, sharedMemoryBytes + startIdx);
                }
            } else {
                if (!isOn) {
                    isOn = true;
                    startIdx = i;
                }
            }
        }

        // Add the final chunk if necessary
        if (isOn) {
            unsigned long length = valueSize - startIdx;
            chunks.emplace_back(startIdx, length, sharedMemoryBytes + startIdx);
        }

        return chunks;
    }
}