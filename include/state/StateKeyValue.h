#pragma once

#include <util/clock.h>
#include <util/exception.h>
#include <redis/Redis.h>

#include <atomic>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>


#define REMOTE_LOCK_TIMEOUT_SECS 1
#define REMOTE_LOCK_MAX_RETRIES 3


namespace state {
    class StateKeyValue {
    public:
        StateKeyValue(const std::string &userIn, const std::string &keyIn, size_t sizeIn);

        const std::string user;

        const std::string key;

        const std::string joinedKey;

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

        void deleteGlobal();

        void pushFull();

        virtual void lockGlobal() = 0;

        virtual void unlockGlobal() = 0;

    protected:
        bool isDirty;

        redis::Redis &redis;

        const std::shared_ptr<spdlog::logger> logger;

        std::shared_mutex valueMutex;
        std::atomic<bool> _fullyAllocated;

        size_t valueSize;
        size_t sharedMemSize;
        void *sharedMemory;
        void *dirtyMask;
        void *allocatedMask;

        void pullImpl(bool onlyIfEmpty);

        void pullSegmentImpl(bool onlyIfEmpty, long offset, size_t length);

        void doPushPartial(const uint8_t *dirtyMaskBytes);

        bool isSegmentAllocated(long offset, size_t length);

        void allocateSegment(long offset, size_t length);

        void zeroDirtyMask();

        void zeroAllocatedMask();

        void initialiseStorage(bool allocate);

        long waitOnRedisRemoteLock(const std::string &redisKey);

        virtual void pullFromRemote() = 0;

        virtual void pullRangeFromRemote(long offset, size_t length) = 0;

        virtual void pushToRemote() = 0;

        virtual void pushPartialToRemote(const uint8_t *dirtyMaskBytes) = 0;

        virtual void deleteFromRemote() = 0;
    };

    class StateKeyValueException : public std::runtime_error {
    public:
        explicit StateKeyValueException(const std::string &message) : runtime_error(message) {

        }
    };
}