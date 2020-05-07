#pragma once

#include <util/clock.h>
#include <util/exception.h>
#include <redis/Redis.h>

#include <atomic>
#include <chrono>
#include <shared_mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>


constexpr int REMOTE_LOCK_TIMEOUT_SECS(1);
constexpr int REMOTE_LOCK_MAX_RETRIES(10);


namespace state {
    class StateChunk {
    public:
        StateChunk(long offsetIn, size_t lengthIn, uint8_t *dataIn) : offset(offsetIn), length(lengthIn),
                                                                      data(dataIn) {

        }

        StateChunk(long offsetIn, std::vector<uint8_t> &data) : offset(offsetIn), length(data.size()),
                                                                      data(data.data()) {

        }

        long offset;
        size_t length;

        // Note - this pointer will always refer to chunks of the underlying
        // state, so does not need to be deleted
        uint8_t *data;
    };


    class StateKeyValue {
    public:
        StateKeyValue(const std::string &userIn, const std::string &keyIn, size_t sizeIn);

        const std::string user;

        const std::string key;

        static uint32_t waitOnRedisRemoteLock(const std::string &redisKey);

        void get(uint8_t *buffer);

        uint8_t *get();

        void getSegment(long offset, uint8_t *buffer, size_t length);

        uint8_t *getSegment(long offset, long len);

        void set(const uint8_t *buffer);

        void setSegment(long offset, const uint8_t *buffer, size_t length);

        void append(const uint8_t *buffer, size_t length);

        void getAppended(uint8_t *buffer, size_t length, long nValues);

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

        size_t size() const;

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

        void markDirtySegment(long offset, long len);

        void markAllocatedSegment(long offset, long len);

        std::vector<StateChunk> getDirtyChunks(const uint8_t *dirtyMaskBytes);

        void doSet(const uint8_t *data);

        void doSetSegment(long offset, const uint8_t *buffer, size_t length);

        virtual void pullFromRemote() = 0;

        virtual void pullRangeFromRemote(long offset, size_t length) = 0;

        virtual void pushToRemote() = 0;

        virtual void appendToRemote(const uint8_t *data, size_t length) = 0;

        virtual void pullAppendedFromRemote(uint8_t *data, size_t length, long nValues) = 0;

        virtual void pushPartialToRemote(const std::vector<StateChunk> &dirtyChunks) = 0;

        virtual void deleteFromRemote() = 0;
    };

    class StateKeyValueException : public std::runtime_error {
    public:
        explicit StateKeyValueException(const std::string &message) : runtime_error(message) {

        }
    };
}