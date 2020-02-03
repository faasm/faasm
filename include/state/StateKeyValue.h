#pragma once

#include <util/clock.h>
#include <util/exception.h>

#include <atomic>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>


namespace state {
    /**
     * A specific key/value pair for state
     */
    class StateKeyValue {
    public:
        // Remote lock timeout in seconds
        unsigned int remoteLockTimeout = 1;

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

        void pushFull();

        void pushPartial();

        void pushPartialMask(const std::shared_ptr<StateKeyValue> &maskKv);

        void clear();

        void lockGlobal();

        void unlockGlobal();

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

        void deleteGlobal();
    private:
        bool isDirty;

        std::shared_mutex valueMutex;

        int lastRemoteLockId = 0;

        size_t valueSize;
        size_t sharedMemSize;
        void *sharedMemory;
        void* dirtyMask;
        void* allocatedMask;

        std::atomic<bool> _fullyAllocated;

        void zeroDirtyMask();

        void zeroAllocatedMask();

        void initialiseStorage(bool allocate);

        void allocateSegment(long offset, size_t length);

        void pullImpl(bool onlyIfEmpty);

        void pullSegmentImpl(bool onlyIfEmpty, long offset, size_t length);

        long waitOnRemoteLock();

        bool isSegmentAllocated(long offset, size_t length);

        void doPushPartial(const uint8_t *dirtyMaskBytes);
    };

    typedef std::unordered_map<std::string, std::shared_ptr<StateKeyValue>> KVMap;
    typedef std::pair<std::string, std::shared_ptr<StateKeyValue>> KVPair;

class StateKeyValueException : public std::runtime_error {
    public:
        explicit StateKeyValueException(const std::string& message): runtime_error(message) {

        }
    };
}