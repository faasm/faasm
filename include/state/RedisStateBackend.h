#pragma once

#include <redis/Redis.h>
#include "StateBackend.h"

namespace state {
    class RedisStateBackend : public StateBackend {
    public:
        RedisStateBackend();

        void get(const std::string &key, uint8_t *buffer, size_t bufferLen) override;

        std::vector<uint8_t> get(const std::string &key) override;

        void getRange(const std::string &key, uint8_t *buffer, size_t bufferLen, long start, long end) override;

        void set(const std::string &key, const uint8_t *value, size_t size) override;

        void set(const std::string &key, const std::vector<uint8_t> &value) override;

        long acquireLock(const std::string &key, int expirySeconds) override;

        void releaseLock(const std::string &key, long lockId) override;

        void del(const std::string &key) override;

        void setRangePipeline(const std::string &key, long offset, const uint8_t *value, size_t size) override;

        void flushPipeline(long pipelineLength) override;

        void enqueueBytes(const std::string &queueName, const std::vector<uint8_t> &value) override;

        void dequeueMultiple(const std::string &queueName, uint8_t *buff, long buffLen, long nElems) override;

    private:
        redis::Redis &redis;
    };
}
