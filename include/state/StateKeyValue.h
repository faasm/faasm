#pragma once

#include <util/clock.h>
#include <util/exception.h>

#include <atomic>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <redis/Redis.h>


namespace state {
    /**
     * A specific key/value pair for state
     */
    class StateKeyValue {
    public:
        // Remote lock timeout in seconds
        int remoteLockTimeout = 1;

        // Remote lock sleep time in milliseconds
        unsigned int remoteLockMaxRetries = 3;

        explicit StateKeyValue(const std::string &keyIn, size_t sizeIn);

        const std::string key;

        void get(uint8_t *buffer);

        uint8_t *get();

        void getSegment(long offset, uint8_t *buffer, size_t length);

        uint8_t *getSegment(long offset, long len);

        void set(const uint8_t *buffer);

        void setSegment(long offset, const uint8_t *buffer, size_t length);

        void mapSharedMemory(void *destination, long pagesOffset, long nPages);

        void unmapSharedMemory(void *mappedAddr);

        void pull();

        void pushPartial();

        void pushPartialMask(const std::shared_ptr<StateKeyValue> &maskKv);

        void clear();

        void lockRead();

        void unlockRead();

        void lockWrite();

        void unlockWrite();

        void flagDirty();

        void zeroValue();

        void flagSegmentDirty(long offset, long len);

        void flagSegmentAllocated(long offset, long len);

        size_t size();

        std::string getSegmentKey(long offset, long length);

        virtual void lockGlobal() = 0;

        virtual void unlockGlobal() = 0;

        virtual void deleteGlobal() = 0;

        virtual void pushFull() = 0;

    protected:
        bool isDirty;

        std::shared_mutex valueMutex;
        std::atomic<bool> _fullyAllocated;

        size_t valueSize;
        size_t sharedMemSize;
        void *sharedMemory;
        void *dirtyMask;
        void *allocatedMask;

        virtual void pullImpl(bool onlyIfEmpty) = 0;

        virtual void pullSegmentImpl(bool onlyIfEmpty, long offset, size_t length) = 0;

        virtual long waitOnRemoteLock() = 0;

        virtual void doPushPartial(const uint8_t *dirtyMaskBytes) = 0;

        bool isSegmentAllocated(long offset, size_t length);

        void allocateSegment(long offset, size_t length);

        void zeroDirtyMask();

        void zeroAllocatedMask();

        void initialiseStorage(bool allocate);
    };

    class RedisStateKeyValue final : public StateKeyValue {
    public:
        RedisStateKeyValue(const std::string &keyIn, size_t sizeIn);

        void pushFull();
    private:
        redis::Redis &redis;

        int lastRemoteLockId = 0;

        void pullImpl(bool onlyIfEmpty);

        void pullSegmentImpl(bool onlyIfEmpty, long offset, size_t length);

        long waitOnRemoteLock();

        void doPushPartial(const uint8_t *dirtyMaskBytes);

        void lockGlobal();

        void unlockGlobal();

        void deleteGlobal();

    };

    // TODO - work out how to do this with different types
    typedef std::unordered_map<std::string, std::shared_ptr<RedisStateKeyValue>> KVMap;
    typedef std::pair<std::string, std::shared_ptr<RedisStateKeyValue>> KVPair;

    class StateKeyValueException : public std::runtime_error {
    public:
        explicit StateKeyValueException(const std::string &message) : runtime_error(message) {

        }
    };
}