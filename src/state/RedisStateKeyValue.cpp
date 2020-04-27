#include "RedisStateKeyValue.h"

#include <util/logging.h>
#include <util/timing.h>
#include <util/macros.h>

namespace state {
    RedisStateKeyValue::RedisStateKeyValue(const std::string &userIn, const std::string &keyIn, size_t sizeIn)
            : StateKeyValue(userIn, keyIn, sizeIn) {

    };

    void RedisStateKeyValue::lockGlobal() {
        lastRemoteLockId = waitOnRedisRemoteLock(joinedKey);
    }

    void RedisStateKeyValue::unlockGlobal() {
        redis.releaseLock(joinedKey, lastRemoteLockId);
    }

    void RedisStateKeyValue::pullFromRemote() {
        PROF_START(statePull)

        // Read from the remote
        logger->debug("Pulling remote value for {}", joinedKey);
        auto memoryBytes = static_cast<uint8_t *>(sharedMemory);
        redis.get(joinedKey, memoryBytes, valueSize);

        PROF_END(statePull)
    }

    void RedisStateKeyValue::pullRangeFromRemote(long offset, size_t length) {
        PROF_START(stateSegmentPull)

        // Note - redis ranges are inclusive, so we need to knock one off
        size_t rangeEnd = offset + length - 1;

        // Read from the remote
        logger->debug("Pulling remote segment ({}-{}) for {}", offset, offset + length, joinedKey);
        auto memoryBytes = static_cast<uint8_t *>(sharedMemory);
        redis.getRange(joinedKey, memoryBytes + offset, length, offset, rangeEnd);

        PROF_END(stateSegmentPull)
    }

    void RedisStateKeyValue::pushToRemote() {
        PROF_START(pushFull)

        logger->debug("Pushing whole value for {}", joinedKey);

        redis.set(joinedKey, static_cast<uint8_t *>(sharedMemory), valueSize);

        // Remove any dirty flags
        isDirty = false;
        zeroDirtyMask();

        PROF_END(pushFull)
    }

    void RedisStateKeyValue::pushPartialToRemote(const uint8_t *dirtyMaskBytes) {
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
                    redis.setRangePipeline(joinedKey, startIdx, sharedMemoryBytes + startIdx, length);
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
            redis.setRangePipeline(joinedKey, startIdx, sharedMemoryBytes + startIdx, length);
            updateCount++;
        }

        // Zero the mask now that we're finished with it
        memset((void *) dirtyMaskBytes, 0, valueSize);

        // Flush the pipeline
        logger->debug("Pipelined {} updates on {}", updateCount, joinedKey);
        redis.flushPipeline(updateCount);

        // Read the latest value
        if (_fullyAllocated) {
            logger->debug("Pulling from remote on partial push for {}", joinedKey);
            redis.get(joinedKey, sharedMemoryBytes, valueSize);
        }

        // Mark as no longer dirty
        isDirty = false;

        PROF_END(pushPartial)
    }

    void RedisStateKeyValue::deleteFromRemote() {
        redis.del(joinedKey);
    }
}