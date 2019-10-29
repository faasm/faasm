#include "StateKeyValue.h"

#include <redis/Redis.h>
#include <util/config.h>
#include <util/memory.h>
#include <util/locks.h>
#include <util/logging.h>
#include <util/timing.h>

#include <sys/mman.h>

using namespace util;

#define PIPELINE

namespace state {
    /**
     * Key/value
     */
    StateKeyValue::StateKeyValue(const std::string &keyIn, size_t sizeIn) : key(keyIn),
                                                                            valueSize(sizeIn) {

        // Work out size of required shared memory
        size_t nHostPages = getRequiredHostPages(valueSize);
        sharedMemSize = nHostPages * HOST_PAGE_SIZE;
        sharedMemory = nullptr;

        isDirty = false;
        _empty = true;
    }

    void StateKeyValue::pull() {
        const std::shared_ptr<spdlog::logger> &logger = getLogger();
        logger->debug("Pulling state for {}", key);
        pullImpl(false);
    }

    void StateKeyValue::pullImpl(bool onlyIfEmpty) {
        // Drop out if we already have the data and we don't care about updating
        {
            SharedLock lock(valueMutex);
            if (onlyIfEmpty && !_empty) {
                return;
            }
        }

        PROF_START(statePull)

        // Unique lock on the whole value
        FullLock lock(valueMutex);

        // Check condition again
        if (onlyIfEmpty && !_empty) {
            return;
        }

        // Initialise the storage if empty
        if (_empty) {
            initialiseStorage();
        }

        // Read from the remote
        const std::shared_ptr<spdlog::logger> &logger = getLogger();
        logger->debug("Pulling remote value for {}", key);

        redis::Redis &redis = redis::Redis::getState();
        redis.get(key, static_cast<uint8_t *>(sharedMemory), valueSize);

        _empty = false;

        PROF_END(statePull)
    }

    long StateKeyValue::waitOnRemoteLock() {
        PROF_START(remoteLock)
        const std::shared_ptr<spdlog::logger> &logger = getLogger();

        redis::Redis &redis = redis::Redis::getState();

        long remoteLockId = redis.acquireLock(key, remoteLockTimeout);
        unsigned int retryCount = 0;
        while (remoteLockId <= 0) {
            logger->debug("Waiting on remote lock for {} (loop {})", key, retryCount);

            if (retryCount >= remoteLockMaxRetries) {
                logger->error("Timed out waiting for lock on {}", key);
                break;
            }

            // Sleep for 1ms
            usleep(1000);

            remoteLockId = redis.acquireLock(key, remoteLockTimeout);
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
        pullImpl(true);

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
        pullImpl(true);

        SharedLock lock(valueMutex);

        uint8_t *segmentPtr = static_cast<uint8_t *>(sharedMemory) + offset;
        return segmentPtr;
    }

    void StateKeyValue::set(const uint8_t *buffer) {
        // Unique lock for setting the whole value
        FullLock lock(valueMutex);

        if (sharedMemory == nullptr) {
            initialiseStorage();
        }

        // Copy data into shared region
        std::copy(buffer, buffer + valueSize, static_cast<uint8_t *>(sharedMemory));
        isDirty = true;
        _empty = false;
    }

    void StateKeyValue::setSegment(long offset, const uint8_t *buffer, size_t length) {
        const std::shared_ptr<spdlog::logger> &logger = getLogger();

        // Check we're in bounds
        size_t end = offset + length;
        if (end > valueSize) {
            logger->error("Trying to write segment finishing at {} (value length {})", end, valueSize);
            throw std::runtime_error("Attempting to set segment out of bounds");
        }

        // If empty, set to full size
        if (sharedMemory == nullptr) {
            FullLock lock(valueMutex);

            if (sharedMemory == nullptr) {
                initialiseStorage();
                _empty = false;
            }
        }

        // Check size
        if (offset + length > valueSize) {
            logger->error("Segment length {} at offset {} too big for size {}", length, offset, valueSize);
            throw std::runtime_error("Setting state segment too big for container");
        }

        // Copy data into shared region
        {
            SharedLock lock(valueMutex);
            auto bytePtr = static_cast<uint8_t *>(sharedMemory);
            std::copy(buffer, buffer + length, bytePtr + offset);
        }

        flagSegmentDirty(offset, length);
    }

    void StateKeyValue::flagDirty() {
        isDirty = true;
    }

    void StateKeyValue::zeroDirtyMask() {
        memset(dirtyMask, 0, valueSize);
    }

    void StateKeyValue::zeroValue() {
        util::FullLock lock(valueMutex);

        memset(sharedMemory, 0, valueSize);
    }

    void StateKeyValue::flagSegmentDirty(long offset, long len) {
        isDirty |= true;
        memset(((uint8_t *) dirtyMask) + offset, 0b11111111, len);
    }

    void StateKeyValue::clear() {
        // Unique lock on the whole value while clearing
        FullLock lock(valueMutex);

        const std::shared_ptr<spdlog::logger> &logger = getLogger();
        logger->debug("Clearing value {}", key);

        // Set flag to say this is effectively new again
        _empty = true;
    }

    bool StateKeyValue::empty() {
        return _empty;
    }

    size_t StateKeyValue::size() {
        return valueSize;
    }

    void StateKeyValue::mapSharedMemory(void *newAddr) {
        pullImpl(true);

        PROF_START(mapSharedMem)

        const std::shared_ptr<spdlog::logger> &logger = getLogger();

        if (!isPageAligned(newAddr)) {
            logger->error("Attempting to map non-page-aligned memory at {} for {}", newAddr, key);
            throw std::runtime_error("Mapping misaligned shared memory");
        }

        FullLock lock(valueMutex);

        // Remap our existing shared memory onto this new region
        void *result = mremap(sharedMemory, 0, sharedMemSize, MREMAP_FIXED | MREMAP_MAYMOVE, newAddr);
        if (result == MAP_FAILED) {
            logger->error("Failed to map shared memory for {} at {} with size {}. errno: {}",
                          key, sharedMemory, sharedMemSize, errno);

            throw std::runtime_error("Failed mapping shared memory");
        }

        if (newAddr != result) {
            logger->error("New mapped addr doesn't match required {} != {}",
                          newAddr, result);

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

    void StateKeyValue::initialiseStorage() {
        PROF_START(initialiseStorage)

        const std::shared_ptr<spdlog::logger> &logger = getLogger();

        if (sharedMemSize == 0) {
            throw StateKeyValueException("Initialising storage without a size for " + key);
        }

        // Create shared memory region for the value
        sharedMemory = mmap(nullptr, sharedMemSize, PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if (sharedMemory == MAP_FAILED) {
            logger->debug("Mmapping of storage size {} failed. errno: {}", sharedMemSize, errno);

            throw std::runtime_error("Failed mapping memory for KV");
        }

        logger->debug("Mmapped {} pages of shared storage for {}", sharedMemSize / HOST_PAGE_SIZE, key);

        // Set up dirty flags
        dirtyMask = new uint8_t[valueSize];
        zeroDirtyMask();

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

        redis::Redis &redis = redis::Redis::getState();
        redis.set(key, static_cast<uint8_t *>(sharedMemory), valueSize);

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
        auto dirtyMaskBytes = reinterpret_cast<uint8_t *>(dirtyMask);
        doPushPartial(dirtyMaskBytes);
    }

    void StateKeyValue::doPushPartial(const uint8_t *dirtyMaskBytes) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Ignore if not dirty
        if (!isDirty) {
            return;
        }

        PROF_START(pushPartial)

        redis::Redis &redis = redis::Redis::getState();

        // We need a full lock while doing this, mainly to ensure no other threads start
        // the same process
        FullLock lock(valueMutex);

#ifdef PIPELINE
        // Double check condition
        if (!isDirty) {
            logger->debug("Ignoring partial push on {}", key);
            return;
        }
        // Iterate through and pipeline the dirty segments
        auto sharedMemoryBytes = reinterpret_cast<uint8_t *>(sharedMemory);
        long updateCount = 0;
        long startIdx = 0;
        bool isOn = false;

        for (size_t i = 0; i < valueSize; i++) {
            if (dirtyMaskBytes[i] == 0) {
                // If we encounter an "off" mask and we're "on", switch off and write the segment
                if(isOn) {
                    isOn = false;

                    // Pipeline the change
                    unsigned long length = i - startIdx;
                    redis.setRangePipeline(key, startIdx, sharedMemoryBytes + startIdx, length);
                    updateCount++;
                }
            } else {
                if(!isOn) {
                    isOn = true;
                    startIdx = i;
                }
            }
        }

        // Write a final chunk
        if (isOn) {
            unsigned long length = valueSize - startIdx;
            redis.setRangePipeline(key, startIdx, sharedMemoryBytes + startIdx, length);
            updateCount++;
        }

        // Zero the mask now that we're finished with it
        memset((void *) dirtyMaskBytes, 0, valueSize);

        // Flush the pipeline
        logger->debug("Pipelined {} updates on {}", updateCount, key);
        redis.flushPipeline(updateCount);

        // Read the latest value
        logger->debug("Pulling from remote for {}", key);
        redis.get(key, sharedMemoryBytes, valueSize);

#else
        // Double check condition
        if (!isDirty) {
            return;
        }
        // Attempt to lock the value remotely
        long remoteLockId = waitOnRemoteLock();

        // Double check condition again after getting remote lock
        if (!isDirty) {
            return;
        }

        // If we don't get remote lock, just skip this push and wait for next one
        const std::shared_ptr<spdlog::logger> &logger = getLogger();
        if (remoteLockId <= 0) {
            logger->debug("Failed to acquire remote lock for {}", key);
            return;
        }

        // Load remote version
        auto tempBuff = new uint8_t[valueSize];
        redis.get(key, tempBuff, valueSize);

        // Take the remote value for any non-dirty parts of our copy
        auto sharedMemBytes = reinterpret_cast<uint8_t *>(sharedMemory);
        for (unsigned long i = 0; i < valueSize; i++) {
            sharedMemBytes[i] = (sharedMemBytes[i] & dirtyMaskBytes[i]) + (tempBuff[i] & ~dirtyMaskBytes[i]);
        }

        delete[] tempBuff;

        // Set whole value back again
        logger->debug("Pushing partial state for {}", key);
        redis.set(key, sharedMemBytes, valueSize);

        // Release remote lock
        redis.releaseLock(key, remoteLockId);
        logger->debug("Released remote lock for {} with id {}", key, remoteLockId);

        // Zero the mask
        memset((void *) dirtyMaskBytes, 0, valueSize);
#endif
        // Mark as no longer dirty
        isDirty = false;

        PROF_END(pushPartial)
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