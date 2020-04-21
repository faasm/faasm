#include "InMemoryStateBackend.h"

namespace state {
    InMemoryStateBackend::InMemoryStateBackend() = default;

    size_t InMemoryStateBackend::getSize(const std::string &key) {
        // TODO - implement
        return 0;
    }

    void InMemoryStateBackend::get(const std::string &key, uint8_t *buffer, size_t bufferLen) {
        // TODO - implement
    }

    std::vector<uint8_t> InMemoryStateBackend::get(const std::string &key) {
        // TODO - implement
        std::vector<uint8_t> empty;
        return empty;
    }

    void InMemoryStateBackend::getRange(const std::string &key, uint8_t *buffer, size_t bufferLen,
                                        long start, long end) {
        // TODO - implement
    }

    void InMemoryStateBackend::set(const std::string &key, const uint8_t *value, size_t size) {
        // TODO - implement
    }

    void InMemoryStateBackend::set(const std::string &key, const std::vector<uint8_t> &value) {
        // TODO - implement
    }

    long InMemoryStateBackend::acquireLock(const std::string &key, int expirySeconds) {
        // TODO - implement
        return 0;
    }

    void InMemoryStateBackend::releaseLock(const std::string &key, long lockId) {
        // TODO - implement
    }

    void InMemoryStateBackend::del(const std::string &key) {
        // TODO - implement
    }

    void
    InMemoryStateBackend::setRangePipeline(const std::string &key, long offset, const uint8_t *value, size_t size) {
        // TODO - implement
    }

    void InMemoryStateBackend::flushPipeline(long pipelineLength) {
        // TODO - implement
    }

    void InMemoryStateBackend::enqueueBytes(const std::string &queueName, const std::vector<uint8_t> &value) {
        // TODO - implement
    }

    void InMemoryStateBackend::dequeueMultiple(const std::string &queueName, uint8_t *buff, long buffLen, long nElems) {
        // TODO - implement
    }

    long InMemoryStateBackend::incrByLong(const std::string &key, long value) {
        // TODO - implement
        return 0;
    }

    void InMemoryStateBackend::setLong(const std::string &key, long value) {
        // TODO - implement
    }

    long InMemoryStateBackend::getLong(const std::string &key) {
        // TODO - implement
        return 0;
    }
}