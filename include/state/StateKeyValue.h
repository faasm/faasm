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

        void mapSharedMemory(void *newAddr, long offset, size_t length);

        void unmapSharedMemory(void *mappedAddr);

        void pull();

        void pushFull();

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

        bool empty();

        size_t size();

    private:
        bool isDirty;

        std::shared_mutex valueMutex;

        size_t valueSize;
        size_t sharedMemSize;
        void *sharedMemory;
        void* dirtyMask;

        std::atomic<bool> _empty;

        void zeroDirtyMask();

        void initialiseStorage(bool allocate);

        void allocateSegment(long offset, size_t length);

        void pullImpl(bool onlyIfEmpty, long offset, size_t length);

        long waitOnRemoteLock();

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