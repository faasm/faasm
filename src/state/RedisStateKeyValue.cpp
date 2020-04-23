#include "RedisStateKeyValue.h"

#include <redis/Redis.h>
#include <util/locks.h>
#include <util/logging.h>
#include <util/timing.h>
#include <util/macros.h>

#define REMOTE_LOCK_TIMEOUT_SECS 1
#define REMOTE_LOCK_MAX_RETRIES 3

namespace state {
    RedisStateKeyValue::RedisStateKeyValue(const std::string &keyIn, size_t sizeIn) : StateKeyValue(keyIn, sizeIn),
                                                                                      redis(redis::Redis::getState()) {
    };

    long RedisStateKeyValue::waitOnRemoteLock() {
        PROF_START(remoteLock)
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        long remoteLockId = redis.acquireLock(key, REMOTE_LOCK_TIMEOUT_SECS);
        unsigned int retryCount = 0;
        while (remoteLockId <= 0) {
            logger->debug("Waiting on remote lock for {} (loop {})", key, retryCount);

            if (retryCount >= REMOTE_LOCK_MAX_RETRIES) {
                logger->error("Timed out waiting for lock on {}", key);
                break;
            }

            // Sleep for 1ms
            usleep(1000);

            remoteLockId = redis.acquireLock(key, REMOTE_LOCK_TIMEOUT_SECS);
            retryCount++;
        }

        PROF_END(remoteLock)
        return remoteLockId;
    }

    void RedisStateKeyValue::lockGlobal() {
        lastRemoteLockId = this->waitOnRemoteLock();
    }

    void RedisStateKeyValue::unlockGlobal() {
        redis.releaseLock(key, lastRemoteLockId);
    }

    void RedisStateKeyValue::pullFromRemote() {
        PROF_START(statePull)
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Read from the remote
        logger->debug("Pulling remote value for {}", key);
        auto memoryBytes = static_cast<uint8_t *>(sharedMemory);
        redis.get(key, memoryBytes, valueSize);

        PROF_END(statePull)
    }

    void RedisStateKeyValue::pullRangeFromRemote(long offset, size_t length) {
        PROF_START(stateSegmentPull)

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Note - redis ranges are inclusive, so we need to knock one off
        size_t rangeEnd = offset + length - 1;

        // Read from the remote
        logger->debug("Pulling remote segment ({}-{}) for {}", offset, offset + length, key);
        auto memoryBytes = static_cast<uint8_t *>(sharedMemory);
        redis.getRange(key, memoryBytes + offset, length, offset, rangeEnd);

        PROF_END(stateSegmentPull)
    }

    void RedisStateKeyValue::pushToRemote() {
        PROF_START(pushFull)

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Pushing whole value for {}", key);

        redis.set(key, static_cast<uint8_t *>(sharedMemory), valueSize);

        // Remove any dirty flags
        isDirty = false;
        zeroDirtyMask();

        PROF_END(pushFull)
    }

    void RedisStateKeyValue::pushPartialToRemote(const uint8_t *dirtyMaskBytes) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        PROF_START(pushPartial)

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

    void RedisStateKeyValue::deleteFromRemote() {
        redis.del(key);
    }
}