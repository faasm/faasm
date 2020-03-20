#pragma once

#include <util/config.h>
#include <vector>

namespace state {
    class StateBackend {
    public:
        /**
         * Fills the buffer with the state value
         */
        virtual void get(const std::string &key, uint8_t* buffer, size_t bufferLen) = 0;

        /**
         * Returns a copy of the state value
         */
        virtual std::vector<uint8_t> get(const std::string &key) = 0;

        /**
         * Fills the buffer with the given subsection of the state value
         */
        virtual void getRange(const std::string &key, uint8_t *buffer, size_t bufferLen, long start, long end) = 0;

        /**
         * Sets the value associated with the given key
         */
        virtual void set(const std::string &key, const uint8_t *value, size_t size) = 0;

        /**
         * Sets the value associated with the given key
         */
        virtual void set(const std::string &key, const std::vector<uint8_t> &value) = 0;

        /**
         * Acquires a global lock
         */
        virtual long acquireLock(const std::string &key, int expirySeconds) = 0;

        /**
         * Releases a global lock
         */
        virtual void releaseLock(const std::string &key, long lockId) = 0;

        /**
         * Deletes the value associated with the key
         */
        virtual void del(const std::string &key) = 0;

        /**
         * Pipelines the operation to set a subsection of the value
         */
        virtual void setRangePipeline(const std::string &key, long offset, const uint8_t *value, size_t size) = 0;

        /**
         * Flushes the pipeline of operations
         */
        virtual void flushPipeline(long pipelineLength) = 0;

        /**
         * Puts the given value on a queue
         */
        virtual void enqueueBytes(const std::string &queueName, const std::vector<uint8_t> &value) = 0;

        /**
         * Dequeues multiple values from a queue
         */
        virtual void dequeueMultiple(const std::string &queueName, uint8_t *buff, long buffLen, long nElems) = 0;
    };
    
    StateBackend &getBackend();
}
