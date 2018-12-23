#pragma once

#include <infra/infra.h>
#include <util/util.h>

#include <exception>
#include <string>
#include <tuple>
#include <thread>
#include <mutex>
#include <shared_mutex>

namespace state {
    typedef std::unique_lock<std::shared_mutex> FullLock;
    typedef std::shared_lock<std::shared_mutex> SharedLock;

    /**
     * A specific key/value pair for state
     */
    class StateKeyValue {
    public:
        explicit StateKeyValue(const std::string &keyIn, size_t sizeIn);

        StateKeyValue(const StateKeyValue& other);

        StateKeyValue& operator=(const StateKeyValue& other);

        const std::string key;

        void get(uint8_t *buffer);

        void getSegment(long offset, uint8_t *buffer, size_t length);

        void set(const uint8_t *buffer);

        void setSegment(long offset, const uint8_t *buffer, size_t length);

        void pull(bool async);

        void pushFull();

        void pushPartial();

        void clear();

        void lockRead();

        void unlockRead();

        void lockWrite();

        void unlockWrite();

        bool empty();

    private:
        std::atomic<bool> isWholeValueDirty;

        std::vector<bool> dirtyFlags;

        std::shared_mutex valueMutex;

        util::TimePoint lastPull;
        util::TimePoint lastInteraction;
        long staleThreshold;
        long idleThreshold;

        std::vector<uint8_t> value;
        size_t size;

        std::atomic<bool> _empty;

        void doRemoteRead();

        void updateLastInteraction();

        bool isStale(const util::TimePoint &now);

        bool isIdle(const util::TimePoint &now);
    };

    typedef std::map<std::string, StateKeyValue *> KVMap;
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

    typedef std::map<std::string, UserState *> UserStateMap;
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