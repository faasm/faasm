#include "StateKeyValue.h"

#include <redis/Redis.h>
#include <util/locks.h>
#include <util/logging.h>
#include <util/timing.h>
#include <util/macros.h>


namespace state {
    RedisStateKeyValue::RedisStateKeyValue(const std::string &keyIn, size_t sizeIn) : StateKeyValue(keyIn, sizeIn),
                                                                                      redis(redis::Redis::getState()) {

    };

    void RedisStateKeyValue::pullImpl(bool onlyIfEmpty) {
        // Drop out if we already have the data and we don't care about updating
        {
            util::SharedLock lock(valueMutex);
            if (onlyIfEmpty && _fullyAllocated) {
                return;
            }
        }

        // Unique lock on the whole value
        util::FullLock lock(valueMutex);
        PROF_START(statePull)

        if (onlyIfEmpty && _fullyAllocated) {
            return;
        }

        // Initialise storage if necessary
        if (!_fullyAllocated) {
            initialiseStorage(true);
        }

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Read from the remote
        logger->debug("Pulling remote value for {}", key);
        auto memoryBytes = static_cast<uint8_t *>(sharedMemory);
        redis.get(key, memoryBytes, valueSize);

        PROF_END(statePull)
    }

    void RedisStateKeyValue::pullSegmentImpl(bool onlyIfEmpty, long offset, size_t length) {
        // Drop out if we already have the data and we don't care about updating
        {
            util::SharedLock lock(valueMutex);
            if (onlyIfEmpty && (_fullyAllocated || isSegmentAllocated(offset, length))) {
                return;
            }
        }

        // Unique lock
        util::FullLock lock(valueMutex);
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

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Note - redis ranges are inclusive, so we need to knock one off
        size_t rangeEnd = offset + length - 1;

        // Read from the remote
        logger->debug("Pulling remote segment ({}-{}) for {}", offset, offset + length, key);
        auto memoryBytes = static_cast<uint8_t *>(sharedMemory);
        redis.getRange(key, memoryBytes + offset, length, offset, rangeEnd);

        PROF_END(stateSegmentPull)
    }


    long RedisStateKeyValue::waitOnRemoteLock() {
        PROF_START(remoteLock)
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

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

    void RedisStateKeyValue::doPushPartial(const uint8_t *dirtyMaskBytes) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Ignore if not dirty
        if (!isDirty) {
            return;
        }

        PROF_START(pushPartial)

        // We need a full lock while doing this, mainly to ensure no other threads start
        // the same process
        util::FullLock lock(valueMutex);

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
                    redis.setRangePipeline(key, startIdx, sharedMemoryBytes + startIdx, length);
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
            redis.setRangePipeline(key, startIdx, sharedMemoryBytes + startIdx, length);
            updateCount++;
        }

        // Zero the mask now that we're finished with it
        memset((void *) dirtyMaskBytes, 0, valueSize);

        // Flush the pipeline
        logger->debug("Pipelined {} updates on {}", updateCount, key);
        redis.flushPipeline(updateCount);

        // Read the latest value
        if (_fullyAllocated) {
            logger->debug("Pulling from remote on partial push for {}", key);
            redis.get(key, sharedMemoryBytes, valueSize);
        }

        // Mark as no longer dirty
        isDirty = false;

        PROF_END(pushPartial)
    }

    void RedisStateKeyValue::deleteGlobal() {
        // Clear locally
        clear();

        // Delete remote
        util::FullLock lock(valueMutex);
        redis.del(key);
    }

    void RedisStateKeyValue::pushFull() {
        // Ignore if not dirty
        if (!isDirty) {
            return;
        }

        PROF_START(pushFull)

        // Get full lock for complete push
        util::FullLock fullLock(valueMutex);

        // Double check condition
        if (!isDirty) {
            return;
        }

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Pushing whole value for {}", key);

        redis.set(key, static_cast<uint8_t *>(sharedMemory), valueSize);

        // Remove any dirty flags
        isDirty = false;
        zeroDirtyMask();

        PROF_END(pushFull)
    }

    void RedisStateKeyValue::lockGlobal() {
        lastRemoteLockId = this->waitOnRemoteLock();
    }

    void RedisStateKeyValue::unlockGlobal() {
        redis.releaseLock(key, lastRemoteLockId);
    }
}