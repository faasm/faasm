#include "RedisStateKeyValue.h"

#include <util/logging.h>
#include <util/timing.h>
#include <util/macros.h>
#include <util/state.h>

namespace state {
    RedisStateKeyValue::RedisStateKeyValue(const std::string &userIn, const std::string &keyIn, size_t sizeIn)
            : StateKeyValue(userIn, keyIn, sizeIn),
              joinedKey(util::keyForUser(user, key)) {

    };

    RedisStateKeyValue::RedisStateKeyValue(const std::string &userIn, const std::string &keyIn)
            : RedisStateKeyValue(userIn, keyIn, 0) {

    };

    size_t RedisStateKeyValue::getStateSizeFromRemote(const std::string &userIn, const std::string &keyIn) {
        std::string actualKey = util::keyForUser(userIn, keyIn);
        redis::Redis &redis = redis::Redis::getState();
        return redis.strlen(actualKey);
    }

    void RedisStateKeyValue::deleteFromRemote(const std::string &userIn, const std::string &keyIn) {
        redis::Redis &redis = redis::Redis::getState();
        std::string actualKey = util::keyForUser(userIn, keyIn);
        redis.del(actualKey);
    }

    void RedisStateKeyValue::clearAll(bool global) {
        if (global) {
            redis::Redis &redis = redis::Redis::getState();
            redis.flushAll();
        }
    }

    // TODO - the remote locking here is quite primitive since we ignore the fact threads can run
    // on the same machine. Redis is also aware of scheduling and so we could optimise this.
    void RedisStateKeyValue::lockGlobal() {
        util::FullLock lock(valueMutex);
        lastRemoteLockId = waitOnRedisRemoteLock(joinedKey);
    }

    void RedisStateKeyValue::unlockGlobal() {
        util::FullLock lock(valueMutex);
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

    void RedisStateKeyValue::pullChunkFromRemote(long offset, size_t length) {
        PROF_START(stateSegmentPull)

        logger->debug("Pulling remote segment ({}-{}) for {}", offset, offset + length, joinedKey);

        // Note - redis ranges are inclusive, so we need to knock one off
        size_t rangeStart = offset;
        size_t rangeEnd = offset + length - 1;

        // Read from the remote
        auto buffer = BYTES(sharedMemory) + offset;
        redis.getRange(joinedKey, buffer, length, rangeStart, rangeEnd);

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

    void RedisStateKeyValue::pushPartialToRemote(const std::vector<StateChunk> &chunks) {
        PROF_START(pushPartial)

        // Pipeline the updates
        for (auto &c : chunks) {
            redis.setRangePipeline(joinedKey, c.offset, c.data, c.length);
        }

        // Flush the pipeline
        logger->debug("Pipelined {} updates on {}", chunks.size(), joinedKey);
        redis.flushPipeline(chunks.size());

        // Read the latest value
        if (fullyAllocated) {
            logger->debug("Pulling from remote on partial push for {}", joinedKey);
            auto sharedMemoryBytes = BYTES(sharedMemory);
            redis.get(joinedKey, sharedMemoryBytes, valueSize);
        }

        PROF_END(pushPartial)
    }

    void RedisStateKeyValue::appendToRemote(const uint8_t *data, size_t length) {
        redis.enqueueBytes(joinedKey, data, length);
    }

    void RedisStateKeyValue::pullAppendedFromRemote(uint8_t *data, size_t length, long nValues) {
        redis.dequeueMultiple(joinedKey, data, length, nValues);
    }

    void RedisStateKeyValue::clearAppendedFromRemote() {
        redis.del(joinedKey);
    }
}