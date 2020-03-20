#pragma once

#include <util/config.h>

namespace state {
    class StateBackend {
    public:
        virtual void get(const std::string &key, uint8_t* buffer, size_t bufferLen) = 0;

        virtual void getRange(const std::string &key, uint8_t *buffer, size_t bufferLen, long start, long end) = 0;

        virtual void set(const std::string &key, const uint8_t *value, size_t size) = 0;

        virtual long acquireLock(const std::string &key, int expirySeconds) = 0;

        virtual void releaseLock(const std::string &key, long lockId) = 0;

        virtual void del(const std::string &key) = 0;

        virtual void setRangePipeline(const std::string &key, long offset, const uint8_t *value, size_t size) = 0;

        virtual void flushPipeline(long pipelineLength) = 0;
    };
    
    StateBackend &getBackend();
}
