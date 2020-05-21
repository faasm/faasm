#include "StateKeyValue.h"

#include <util/config.h>
#include <util/memory.h>
#include <util/locks.h>
#include <util/logging.h>
#include <util/timing.h>

#include <sys/mman.h>
#include <util/macros.h>

using namespace util;

#define ONES_BITMASK 0b11111111

namespace state {
    StateKeyValue::StateKeyValue(const std::string &userIn, const std::string &keyIn) :
            StateKeyValue(userIn, keyIn, 0) {
        // If using this constructor, we don't know the size, hence cannot use
        // a number of operations.
    }

    StateKeyValue::StateKeyValue(const std::string &userIn, const std::string &keyIn, size_t sizeIn) :
            user(userIn),
            key(keyIn),
            redis(redis::Redis::getState()),
            logger(util::getLogger()),
            valueSize(sizeIn) {

        if (sizeIn > 0) {
            configureSize();
        }
    }

    void StateKeyValue::configureSize() {
        // Work out size of required shared memory
        size_t nHostPages = getRequiredHostPages(valueSize);
        sharedMemSize = nHostPages * HOST_PAGE_SIZE;
        sharedMemory = nullptr;

        dirtyMask = new uint8_t[valueSize];
        zeroDirtyMask();

        pulledMask = new uint8_t[valueSize];
        memset(pulledMask, 0, valueSize);
    }

    void StateKeyValue::checkSizeConfigured() {
        if (valueSize <= 0) {
            throw StateKeyValueException(fmt::format("{}/{} has no size set", user, key));
        }
    }

    void StateKeyValue::pull() {
        logger->debug("Pulling state for {}/{}", user, key);
        doPull(false);
    }

    bool StateKeyValue::isChunkPulled(long offset, size_t length) {
        checkSizeConfigured();

        if (fullyPulled) {
            return true;
        }

        // TODO - more efficient way of checking this
        auto pulledMaskBytes = BYTES(pulledMask);
        for (size_t i = 0; i < length; i++) {
            if (pulledMaskBytes[offset + i] == 0) {
                return false;
            }
        }

        return true;
    }

    void StateKeyValue::get(uint8_t *buffer) {
        doPull(true);

        SharedLock lock(valueMutex);
        auto bytePtr = BYTES(sharedMemory);
        std::copy(bytePtr, bytePtr + valueSize, buffer);
    }

    uint8_t *StateKeyValue::get() {
        doPull(true);
        return BYTES(sharedMemory);
    }

    void StateKeyValue::getChunk(long offset, uint8_t *buffer, size_t length) {
        doPullChunk(true, offset, length);

        SharedLock lock(valueMutex);
        auto bytePtr = BYTES(sharedMemory);
        std::copy(bytePtr + offset, bytePtr + offset + length, buffer);
    }

    uint8_t *StateKeyValue::getChunk(long offset, long len) {
        doPullChunk(true, offset, len);
        return BYTES(sharedMemory) + offset;
    }

    void StateKeyValue::set(const uint8_t *buffer) {
        checkSizeConfigured();

        // Unique lock for setting the whole value
        FullLock lock(valueMutex);
        doSet(buffer);
        isDirty = true;
    }

    void StateKeyValue::doSet(const uint8_t *buffer) {
        checkSizeConfigured();

        // Set up storage
        allocateFull();

        // Copy data into shared region
        std::copy(buffer, buffer + valueSize, BYTES(sharedMemory));
    }

    void StateKeyValue::append(const uint8_t *buffer, size_t length) {
        FullLock lock(valueMutex);

        appendToRemote(buffer, length);
    }

    void StateKeyValue::getAppended(uint8_t *buffer, size_t length, long nValues) {
        SharedLock lock(valueMutex);

        pullAppendedFromRemote(buffer, length, nValues);
    }

    void StateKeyValue::clearAppended() {
        SharedLock lock(valueMutex);

        clearAppendedFromRemote();
    }

    void StateKeyValue::setChunk(long offset, const uint8_t *buffer, size_t length) {
        checkSizeConfigured();

        FullLock lock(valueMutex);

        doSetChunk(offset, buffer, length);
        markDirtyChunk(offset, length);
    }

    void StateKeyValue::doSetChunk(long offset, const uint8_t *buffer, size_t length) {
        checkSizeConfigured();

        // Check we're in bounds - note that we permit chunks within the _allocated_ memory
        size_t chunkEnd = offset + length;
        if (chunkEnd > sharedMemSize) {
            logger->error("Setting chunk out of bounds on {}/{} ({} > {})", user, key, chunkEnd, valueSize);
            throw std::runtime_error("Attempting to set chunk out of bounds");
        }

        // If necessary, allocate the memory
        allocateChunk(offset, length);

        // Do the copy
        std::copy(buffer, buffer + length, BYTES(sharedMemory) + offset);
    }

    void StateKeyValue::flagDirty() {
        util::SharedLock lock(valueMutex);
        isDirty = true;
    }

    void StateKeyValue::zeroDirtyMask() {
        checkSizeConfigured();
        memset(dirtyMask, 0, valueSize);
    }

    void StateKeyValue::flagChunkDirty(long offset, long len) {
        checkSizeConfigured();

        util::SharedLock lock(valueMutex);
        markDirtyChunk(offset, len);
    }

    void StateKeyValue::markDirtyChunk(long offset, long len) {
        isDirty |= true;
        memset(BYTES(dirtyMask) + offset, ONES_BITMASK, len);
    }

    size_t StateKeyValue::size() const {
        return valueSize;
    }

    size_t StateKeyValue::getSharedMemorySize() const {
        return sharedMemSize;
    }

    void StateKeyValue::mapSharedMemory(void *destination, long pagesOffset, long nPages) {
        checkSizeConfigured();

        PROF_START(mapSharedMem)

        if (!isPageAligned(destination)) {
            logger->error("Non-aligned destination for shared mapping of {}", key);
            throw std::runtime_error("Mapping misaligned shared memory");
        }

        // Ensure the underlying memory is allocated
        size_t offset = pagesOffset * util::HOST_PAGE_SIZE;
        size_t length = nPages * util::HOST_PAGE_SIZE;
        allocateChunk(offset, length);

        // Full lock on the key-value
        FullLock lock(valueMutex);

        // Add a mapping of the relevant pages of shared memory onto the new region
        void *result = mremap(BYTES(sharedMemory) + offset, 0, length, MREMAP_FIXED | MREMAP_MAYMOVE, destination);

        // Handle failure
        if (result == MAP_FAILED) {
            logger->error("Failed mapping for {} at {} with size {}. errno: {} ({})",
                          key, offset, length, errno, strerror(errno));

            throw std::runtime_error("Failed mapping shared memory");
        }

        // Check the mapping is where we expect it to be
        if (destination != result) {
            logger->error("New mapped addr for {} doesn't match required {} != {}", key, destination, result);
            throw std::runtime_error("Misaligned shared memory mapping");
        }

        PROF_END(mapSharedMem)
    }

    void StateKeyValue::unmapSharedMemory(void *mappedAddr) {
        FullLock lock(valueMutex);

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

    void StateKeyValue::allocateChunk(long offset, size_t length) {
        // Can skip if the whole thing is already allocated
        if (fullyAllocated) {
            return;
        }

        // Ensure storage is reserved
        reserveStorage();

        // Page-align the chunk
        AlignedChunk chunk = getPageAlignedChunk(offset, length);

        // Make sure all the pages involved are writable
        int res = mprotect(BYTES(sharedMemory) + chunk.nBytesOffset, chunk.nBytesLength, PROT_WRITE);
        if (res != 0) {
            logger->debug("Allocating memory for {}/{} of size {} failed: {} ({})", user, key, length, errno, strerror(errno));
            throw std::runtime_error("Failed allocating memory for KV");
        }
    }

    void StateKeyValue::allocateFull() {
        // Skip if already done
        if (fullyAllocated) {
            return;
        }

        // Allocate full memory (i.e. a chunk from zero to max)
        allocateChunk(0, sharedMemSize);

        fullyAllocated = true;
    }

    void StateKeyValue::reserveStorage() {
        checkSizeConfigured();

        PROF_START(reserveStorage)

        // Check if already reserved
        if (sharedMemory != nullptr) {
            return;
        }

        if (sharedMemSize == 0) {
            throw StateKeyValueException("Reserving storage with no size for " + key);
        }

        // Create shared memory region with no permissions
        sharedMemory = mmap(nullptr, sharedMemSize, PROT_NONE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if (sharedMemory == MAP_FAILED) {
            logger->debug("Mmapping of storage size {} failed. errno: {}", sharedMemSize, errno);

            throw std::runtime_error("Failed mapping memory for KV");
        }

        size_t nPages = sharedMemSize / HOST_PAGE_SIZE;
        logger->debug("Reserved {} pages of shared storage for {}", nPages, key);

        PROF_END(reserveStorage)
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
        checkSizeConfigured();

        auto dirtyMaskBytes = BYTES(dirtyMask);
        doPushPartial(dirtyMaskBytes);
    }

    void StateKeyValue::pushFull() {
        checkSizeConfigured();

        // Ignore if not dirty
        if (!isDirty) {
            return;
        }

        // Get full lock for complete push
        util::FullLock fullLock(valueMutex);

        // Double check condition
        if (!isDirty) {
            return;
        }

        pushToRemote();

        // Remove any dirty flags
        isDirty = false;
        zeroDirtyMask();
    }

    void StateKeyValue::doPull(bool lazy) {
        checkSizeConfigured();

        // Drop out if we already have the data and we don't care about updating
        {
            util::SharedLock lock(valueMutex);
            if (lazy && fullyPulled) {
                return;
            }
        }

        // Unique lock on the whole value
        util::FullLock lock(valueMutex);

        // Check again if we need to do this
        if (lazy && fullyPulled) {
            return;
        }

        // Make sure storage is allocated
        allocateFull();

        // Do the pull
        pullFromRemote();
        fullyPulled = true;
    }

    void StateKeyValue::doPullChunk(bool lazy, long offset, size_t length) {
        checkSizeConfigured();

        // Check bounds
        size_t chunkEnd = offset + length;
        if (chunkEnd > valueSize) {
            logger->error("Pulling chunk out of bounds on {}/{} ({} > {})", user, key, chunkEnd, valueSize);
            throw std::runtime_error("Out of bounds chunk");
        }

        // Drop out if we already have the data and we don't care about updating
        {
            util::SharedLock lock(valueMutex);
            if (lazy && (fullyAllocated || isChunkPulled(offset, length))) {
                return;
            }
        }

        // Unique lock
        util::FullLock lock(valueMutex);

        // Check condition again
        bool chunkPulled = isChunkPulled(offset, length);
        if (lazy && (fullyAllocated || chunkPulled)) {
            return;
        }

        // Allocate this chunk
        allocateChunk(offset, length);
        pullChunkFromRemote(offset, length);

        // Mark the chunk as pulled
        memset(((uint8_t *) pulledMask) + offset, ONES_BITMASK, length);
    }

    void StateKeyValue::doPushPartial(const uint8_t *dirtyMaskBytes) {
        // Ignore if not dirty
        if (!isDirty) {
            return;
        }

        // We need a full lock while doing this, mainly to ensure no other threads start
        // the same process
        util::FullLock lock(valueMutex);

        // Double check condition
        if (!isDirty) {
            logger->debug("No need for partial push on {}", key);
            return;
        }

        // Work out what's dirty
        const std::vector<StateChunk> &chunks = getDirtyChunks(dirtyMaskBytes);

        // Zero the mask now that we're finished with it
        memset((void *) dirtyMaskBytes, 0, valueSize);

        // Push
        pushPartialToRemote(chunks);

        // Update if necessary
        if(fullyAllocated) {
            pullFromRemote();
        }

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

            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            remoteLockId = redis.acquireLock(redisKey, REMOTE_LOCK_TIMEOUT_SECS);
            retryCount++;
        }

        PROF_END(remoteLock)
        return remoteLockId;
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

    std::vector<StateChunk> StateKeyValue::getDirtyChunks(const uint8_t *dirtyMaskBytes) {
        std::vector<StateChunk> chunks;

        auto sharedMemoryBytes = BYTES(sharedMemory);
        long startIdx = 0;
        bool isOn = false;

        // Iterate through looking for dirty chunks
        for (size_t i = 0; i < valueSize; i++) {
            if (dirtyMaskBytes[i] == 0) {
                // If we encounter an "off" mask and we're "on", switch off and write the chunk
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