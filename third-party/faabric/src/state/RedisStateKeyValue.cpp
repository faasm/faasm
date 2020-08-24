#include "RedisStateKeyValue.h"

#include <faabric/util/logging.h>
#include <faabric/util/timing.h>
#include <faabric/util/macros.h>
#include <faabric/util/state.h>

/**
 * WARNING - key-value objects are shared between threads, BUT
 * hiredis is not thread-safe, so make sure you always retrieve
 * the reference to Redis inline rather than sharing a reference
 * within the class.
 */

namespace faabric::state {
    RedisStateKeyValue::RedisStateKeyValue(const std::string &userIn, const std::string &keyIn, size_t sizeIn)
            : StateKeyValue(userIn, keyIn, sizeIn),
              joinedKey(faabric::utilkeyForUser(user, key)) {

    };

    RedisStateKeyValue::RedisStateKeyValue(const std::string &userIn, const std::string &keyIn)
            : RedisStateKeyValue(userIn, keyIn, 0) {

    };

    size_t RedisStateKeyValue::getStateSizeFromRemote(const std::string &userIn, const std::string &keyIn) {
        std::string actualKey = faabric::utilkeyForUser(userIn, keyIn);
        return redis::Redis::getState().strlen(actualKey);
    }

    void RedisStateKeyValue::deleteFromRemote(const std::string &userIn, const std::string &keyIn) {
        std::string actualKey = faabric::utilkeyForUser(userIn, keyIn);
        redis::Redis::getState().del(actualKey);
    }

    void RedisStateKeyValue::clearAll(bool global) {
        if (global) {
            redis::Redis::getState().flushAll();
        }
    }

    // TODO - the remote locking here is quite primitive since we ignore the fact threads can run
    // on the same machine. Redis is also aware of scheduling and so we could optimise this.
    void RedisStateKeyValue::lockGlobal() {
        faabric::utilFullLock lock(valueMutex);
        lastRemoteLockId = waitOnRedisRemoteLock(joinedKey);
    }

    void RedisStateKeyValue::unlockGlobal() {
        faabric::utilFullLock lock(valueMutex);
        redis::Redis::getState().releaseLock(joinedKey, lastRemoteLockId);
    }

    void RedisStateKeyValue::pullFromRemote() {
        PROF_START(statePull)

        // Read from the remote
        logger->debug("Pulling remote value for {}", joinedKey);
        auto memoryBytes = static_cast<uint8_t *>(sharedMemory);
        redis::Redis::getState().get(joinedKey, memoryBytes, valueSize);

        PROF_END(statePull)
    }

    void RedisStateKeyValue::pullChunkFromRemote(long offset, size_t length) {
        PROF_START(stateChunkPull)

        logger->debug("Pulling remote chunk ({}-{}) for {}", offset, offset + length, joinedKey);

        // Note - redis ranges are inclusive, so we need to knock one off
        size_t rangeStart = offset;
        size_t rangeEnd = offset + length - 1;

        // Read from the remote
        auto buffer = BYTES(sharedMemory) + offset;
        redis::Redis::getState().getRange(joinedKey, buffer, length, rangeStart, rangeEnd);

        PROF_END(stateChunkPull)
    }

    void RedisStateKeyValue::pushToRemote() {
        PROF_START(pushFull)

        logger->debug("Pushing whole value for {}", joinedKey);

        redis::Redis::getState().set(joinedKey, static_cast<uint8_t *>(sharedMemory), valueSize);

        PROF_END(pushFull)
    }

    void RedisStateKeyValue::pushPartialToRemote(const std::vector<StateChunk> &chunks) {
        PROF_START(pushPartial)

        redis::Redis &redis = redis::Redis::getState();
        
        // Pipeline the updates
        for (auto &c : chunks) {
            redis.setRangePipeline(joinedKey, c.offset, c.data, c.length);
        }

        // Flush the pipeline
        logger->debug("Pipelined {} updates on {}", chunks.size(), joinedKey);
        redis.flushPipeline(chunks.size());

        PROF_END(pushPartial)
    }

    void RedisStateKeyValue::appendToRemote(const uint8_t *data, size_t length) {
        redis::Redis::getState().enqueueBytes(joinedKey, data, length);
    }

    void RedisStateKeyValue::pullAppendedFromRemote(uint8_t *data, size_t length, long nValues) {
        redis::Redis::getState().dequeueMultiple(joinedKey, data, length, nValues);
    }

    void RedisStateKeyValue::clearAppendedFromRemote() {
        redis::Redis::getState().del(joinedKey);
    }
}