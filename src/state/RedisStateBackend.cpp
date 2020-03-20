#include "RedisStateBackend.h"

namespace state {
    RedisStateBackend::RedisStateBackend() : redis(redis::Redis::getState()) {

    }

    void RedisStateBackend::get(const std::string &key, uint8_t *buffer, size_t bufferLen) {
        redis.get(key, buffer, bufferLen);
    }

    std::vector<uint8_t> RedisStateBackend::get(const std::string &key) {
        std::vector<uint8_t> empty;
        return empty;
    }

    void RedisStateBackend::getRange(const std::string &key, uint8_t *buffer, size_t bufferLen, long start, long end) {
        redis.getRange(key, buffer, bufferLen, start, end);
    }

    void RedisStateBackend::set(const std::string &key, const uint8_t *value, size_t size) {
        redis.set(key, value, size);
    }

    void RedisStateBackend::set(const std::string &key, const std::vector<uint8_t> &value) {
        redis.set(key, value);
    }

    long RedisStateBackend::acquireLock(const std::string &key, int expirySeconds) {
        return redis.acquireLock(key, expirySeconds);
    }

    void RedisStateBackend::releaseLock(const std::string &key, long lockId) {
        redis.releaseLock(key, lockId);
    }

    void RedisStateBackend::del(const std::string &key) {
        redis.del(key);
    }

    void RedisStateBackend::setRangePipeline(const std::string &key, long offset, const uint8_t *value, size_t size) {
        redis.setRangePipeline(key, offset, value, size);
    }

    void RedisStateBackend::flushPipeline(long pipelineLength) {
        redis.flushPipeline(pipelineLength);
    }
}