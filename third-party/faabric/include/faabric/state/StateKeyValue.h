#pragma once

#include <faabric/util/clock.h>
#include <faabric/util/exception.h>
#include <faabric/redis/Redis.h>

#include <atomic>
#include <chrono>
#include <shared_mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>


constexpr int REMOTE_LOCK_TIMEOUT_SECS(1);
constexpr int REMOTE_LOCK_MAX_RETRIES(100);

#define STATE_STREAMING_CHUNK_SIZE (64 * 1024)


namespace faabric::state {
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

        StateKeyValue(const std::string &userIn, const std::string &keyIn);

        const std::string user;

        const std::string key;

        static uint32_t waitOnRedisRemoteLock(const std::string &redisKey);

        void get(uint8_t *buffer);

        uint8_t *get();

        void getChunk(long offset, uint8_t *buffer, size_t length);

        uint8_t *getChunk(long offset, long len);

        std::vector<StateChunk> getAllChunks();

        void set(const uint8_t *buffer);

        void setChunk(long offset, const uint8_t *buffer, size_t length);

        void append(const uint8_t *buffer, size_t length);

        void getAppended(uint8_t *buffer, size_t length, long nValues);

        void clearAppended();

        void mapSharedMemory(void *destination, long pagesOffset, long nPages);

        void unmapSharedMemory(void *mappedAddr);

        void pull();

        void pushPartial();

        void pushPartialMask(const std::shared_ptr<StateKeyValue> &maskKv);

        void lockRead();

        void unlockRead();

        void lockWrite();

        void unlockWrite();

        void flagDirty();

        void flagChunkDirty(long offset, long len);

        size_t size() const;

        size_t getSharedMemorySize() const;

        void pushFull();

        virtual void lockGlobal() = 0;

        virtual void unlockGlobal() = 0;

    protected:
        const std::shared_ptr<spdlog::logger> logger;

        std::shared_mutex valueMutex;

        size_t valueSize;
        size_t sharedMemSize;

        void *sharedMemory = nullptr;

        void doSet(const uint8_t *data);

        void doSetChunk(long offset, const uint8_t *buffer, size_t length);

        virtual void pullFromRemote() = 0;

        virtual void pullChunkFromRemote(long offset, size_t length) = 0;

        virtual void pushToRemote() = 0;

        virtual void appendToRemote(const uint8_t *data, size_t length) = 0;

        virtual void pullAppendedFromRemote(uint8_t *data, size_t length, long nValues) = 0;

        virtual void clearAppendedFromRemote() = 0;

        virtual void pushPartialToRemote(const std::vector<StateChunk> &dirtyChunks) = 0;

    private:
        // Flags for tracking allocation and initial pull
        std::atomic<bool> fullyAllocated = false;
        std::atomic<bool> fullyPulled = false;
        void *pulledMask = nullptr;
        void *dirtyMask = nullptr;
        bool isDirty = false;

        void zeroDirtyMask();

        void configureSize();

        void checkSizeConfigured();

        void markDirtyChunk(long offset, long len);

        bool isChunkPulled(long offset, size_t length);

        void allocateChunk(long offset, size_t length);

        void reserveStorage();

        void doPull(bool lazy);

        void doPullChunk(bool lazy, long offset, size_t length);

        void doPushPartial(const uint8_t *dirtyMaskBytes);

        std::vector<StateChunk> getDirtyChunks(const uint8_t *dirtyMaskBytes);
    };

    class StateKeyValueException : public std::runtime_error {
    public:
        explicit StateKeyValueException(const std::string &message) : runtime_error(message) {

        }
    };
}