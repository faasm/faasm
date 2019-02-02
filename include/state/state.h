#pragma once

#include <util/clock.h>

#include <vector>
#include <atomic>
#include <exception>
#include <string>
#include <tuple>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <map>
#include <shared_mutex>


namespace state {
    /**
     * A specific key/value pair for state
     */
    class StateKeyValue {
    public:
        // Remote lock timeout in seconds
        unsigned int remoteLockTimeout = 5;
        // Remote lock sleep time in milliseconds
        unsigned int remoteLockWaitTime = 500;
        unsigned int remoteLockMaxRetries = 10;

        explicit StateKeyValue(const std::string &keyIn, size_t sizeIn);

        const std::string key;

        void get(uint8_t *buffer);

        uint8_t *get();

        void getSegment(long offset, uint8_t *buffer, size_t length);

        uint8_t *getSegment(long offset, long len);

        void set(const uint8_t *buffer);

        void setSegment(long offset, const uint8_t *buffer, size_t length);

        void mapSharedMemory(void* newAddr);

        void unmapSharedMemory(void *mappedAddr);

        void pull(bool async);

        void pushFull();

        void pushPartial();

        void clear();

        void lockRead();

        void unlockRead();

        void lockWrite();

        void unlockWrite();

        void flagFullValueDirty();

        void flagSegmentDirty(long offset, long len);

        bool empty();

        size_t size();

    private:
        bool isWholeValueDirty;
        bool isPartiallyDirty;
        std::vector<bool> dirtyFlags;

        bool fullAsync;

        std::shared_mutex valueMutex;

        util::TimePoint lastPull;
        util::TimePoint lastInteraction;
        long staleThreshold;
        long idleThreshold;

        size_t valueSize;
        size_t sharedMemSize;
        void *sharedMemory;

        std::atomic<bool> _empty;

        void initialiseStorage();

        void doRemoteRead();

        void updateLastInteraction();

        bool isStale(const util::TimePoint &now);

        bool isIdle(const util::TimePoint &now);

        void preGet();

        long waitOnRemoteLock();
    };

    typedef std::unordered_map<std::string, StateKeyValue *> KVMap;
    typedef std::pair<std::string, StateKeyValue *> KVPair;

    /**
     * Holds state for a given user
     */
    class UserState {
    public:
        explicit UserState(const std::string &userIn);

        ~UserState();

        StateKeyValue *getValue(const std::string &key, size_t size);

        void pushAll();

    private:
        const std::string user;

        KVMap kvMap;
        std::shared_mutex kvMapMutex;
    };

    typedef std::unordered_map<std::string, UserState *> UserStateMap;
    typedef std::pair<std::string, UserState *> UserStatePair;

    /**
     * Global entry point into state
     */
    class State {
    public:
        State();

        ~State();

        StateKeyValue *getKV(const std::string &user, const std::string &key, size_t size);

        UserState *getUserState(const std::string &user);

        void pushLoop();

        void pushAll();

        void forceClearAll();

    private:
        UserStateMap userStateMap;
        std::shared_mutex userStateMapMutex;

        long pushInterval;
    };

    State &getGlobalState();
}