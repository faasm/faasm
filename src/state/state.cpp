#include <algorithm>
#include "state.h"

namespace state {

    /**
     * Key/value
     */
    StateKeyValue::StateKeyValue(const std::string &keyIn, size_t sizeIn) : key(keyIn),
                                                                            size(sizeIn) {

        isWholeValueDirty = false;
        _empty = true;

        // Gets over the stale threshold trigger a pull from remote
        const util::SystemConfig &conf = util::getSystemConfig();
        staleThreshold = conf.stateStaleThreshold;

        // State over the clear threshold is removed from local
        idleThreshold = conf.stateClearThreshold;

        // Whether to run in fully async mode
        fullAsync = conf.fullAsync > 0;
    }

    void StateKeyValue::pull(bool async) {
        this->updateLastInteraction();

        // Check if new (one-off initialisation)
        if (_empty) {
            // Unique lock on the whole value while loading
            FullLock lock(valueMutex);

            // Double check assumption
            if (_empty) {
                const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
                logger->debug("Initialising state for {}", key);

                doRemoteRead();
                _empty = false;
                return;
            }
        }

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if(async && fullAsync) {
            // Never pull in full async mode
        }
        else if (async) {
            // Check staleness
            util::Clock &clock = util::getGlobalClock();
            const util::TimePoint now = clock.now();

            // If stale, try to update from remote
            if (this->isStale(now)) {
                // Unique lock on the whole value while loading
                FullLock lock(valueMutex);

                // Double check staleness
                if (this->isStale(now)) {
                    logger->debug("Refreshing stale state for {}", key);
                    doRemoteRead();
                }
            }
        } else {
            FullLock lock(valueMutex);
            logger->debug("Sync read for state {}", key);
            doRemoteRead();
        }
    }

    void StateKeyValue::doRemoteRead() {
        // Initialise the data array with zeroes
        if (_empty) {
            value.resize(size);
            dirtyFlags.resize(size);
        }

        // Read from the remote
        infra::Redis *redis = infra::Redis::getThreadState();
        redis->get(key, value.data(), size);

        util::Clock &clock = util::getGlobalClock();
        const util::TimePoint now = clock.now();
        lastPull = now;

        this->updateLastInteraction();
    }

    void StateKeyValue::updateLastInteraction() {
        util::Clock &clock = util::getGlobalClock();
        const util::TimePoint now = clock.now();
        lastInteraction = now;
    }

    bool StateKeyValue::isStale(const util::TimePoint &now) {
        util::Clock &clock = util::getGlobalClock();
        long age = clock.timeDiff(now, lastPull);
        return age > staleThreshold;
    }

    bool StateKeyValue::isIdle(const util::TimePoint &now) {
        util::Clock &clock = util::getGlobalClock();
        long idleTime = clock.timeDiff(now, lastInteraction);
        return idleTime > idleThreshold;
    }

    void StateKeyValue::get(uint8_t *buffer) {
        if (this->empty()) {
            throw std::runtime_error("Must pull before accessing state");
        }

        this->updateLastInteraction();

        // Shared lock for full reads
        SharedLock lock(valueMutex);

        std::copy(value.data(), value.data() + size, buffer);
    }

    void StateKeyValue::getSegment(long offset, uint8_t *buffer, size_t length) {
        if (this->empty()) {
            throw std::runtime_error("Must pull before accessing state");
        }

        this->updateLastInteraction();

        SharedLock lock(valueMutex);

        // Return just the required segment
        if ((offset + length) > size) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

            logger->error("Out of bounds read at {} on {} with length {}", offset + length, key, size);
            throw std::runtime_error("Out of bounds read");
        }

        std::copy(value.data() + offset, value.data() + offset + length, buffer);
    }

    void StateKeyValue::set(const uint8_t *buffer) {
        this->updateLastInteraction();

        // Unique lock for setting the whole value
        FullLock lock(valueMutex);

        if (value.empty()) {
            value.resize(size);
            dirtyFlags.resize(size);
        }

        // Copy data into shared region
        std::copy(buffer, buffer + size, value.data());

        isWholeValueDirty = true;
        _empty = false;
    }

    void StateKeyValue::setSegment(long offset, const uint8_t *buffer, size_t length) {
        this->updateLastInteraction();

        // Check we're in bounds
        size_t end = offset + length;
        if (end > size) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->error("Trying to write segment finishing at {} (value length {})", end, size);
            throw std::runtime_error("Attempting to set segment out of bounds");
        }

        // If empty, set to full size
        if (value.empty()) {
            FullLock lock(valueMutex);
            if (value.empty()) {
                value.resize(size);
                dirtyFlags.resize(size);
            }
        }

        // Check size
        if (offset + length > size) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

            logger->error("Segment length {} at offset {} too big for size {}", length, offset, size);
            throw std::runtime_error("Setting state segment too big for container");
        }

        // Copy data into shared region
        SharedLock lock(valueMutex);
        std::copy(buffer, buffer + length, value.data() + offset);

        // Flag as dirty
        std::fill(dirtyFlags.begin() + offset, dirtyFlags.begin() + offset + length, true);
    }

    void StateKeyValue::clear() {
        // Check age since last interaction
        util::Clock &c = util::getGlobalClock();
        util::TimePoint now = c.now();

        // If over clear threshold, remove
        if (this->isIdle(now) && !_empty) {
            // Unique lock on the whole value while clearing
            FullLock lock(valueMutex);

            // Double check still over the threshold
            if (this->isIdle(now)) {
                const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
                logger->debug("Clearing unused value {}", key);

                value.clear();

                // Set flag to say this is effectively new again
                _empty = true;
            }
        }
    }

    bool StateKeyValue::empty() {
        return _empty;
    }

    void StateKeyValue::pushFull() {
        if(fullAsync) {
            throw std::runtime_error("Shouldn't be pushing in full async mode.");
        }

        // Double check condition
        if (!isWholeValueDirty) {
            return;
        }

        // Get full lock for complete write
        FullLock fullLock(valueMutex);

        // Double check condition
        if (!isWholeValueDirty) {
            return;
        }

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Pushing whole value for {}", key);

        infra::Redis *redis = infra::Redis::getThreadState();
        redis->set(key, value.data(), size);

        // Reset (as we're setting the full value, we've effectively pulled)
        util::Clock &clock = util::getGlobalClock();
        lastPull = clock.now();
        isWholeValueDirty = false;

        // Remove any dirty flags
        std::fill(dirtyFlags.begin(), dirtyFlags.end(), false);
    }

    void StateKeyValue::pushPartial() {
        if(fullAsync) {
            throw std::runtime_error("Shouldn't be pushing in full async mode.");
        }

        // Ignore if the whole value is dirty
        if (isWholeValueDirty) {
            return;
        }

        // Create copy of the dirty flags and clear the old version
        std::vector<bool> dirtyFlagsCopy(size);
        {
            FullLock segmentsLock(valueMutex);
            std::copy(dirtyFlags.begin(), dirtyFlags.end(), dirtyFlagsCopy.begin());
            std::fill(dirtyFlags.begin(), dirtyFlags.end(), false);
        }

        // Shared lock for writing segments
        SharedLock sharedLock(valueMutex);

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        infra::Redis *redis = infra::Redis::getThreadState();

        // Go through all dirty flags and update accordingly

        // Find first true flag
        auto start = std::find(dirtyFlagsCopy.begin(), dirtyFlagsCopy.end(), true);

        // While we still have more segments
        while (start != dirtyFlagsCopy.end()) {
            // Find next false
            auto end = std::find(start + 1, dirtyFlagsCopy.end(), false);

            logger->debug("Pushing partial value for {}", key);

            // Our indices are inclusive here
            long size = end - start;
            long offset = start - dirtyFlagsCopy.begin();
            redis->setRange(
                    key,
                    offset,
                    value.data() + offset,
                    (size_t) size
            );

            // Find next true
            start = std::find(end + 1, dirtyFlagsCopy.end(), true);
        }
    }

    void StateKeyValue::lockRead() {
        valueMutex.lock_shared();
    }

    void StateKeyValue::unlockRead() {
        valueMutex.unlock_shared();
    }

    void StateKeyValue::lockWrite() {
        valueMutex.lock();
    }

    void StateKeyValue::unlockWrite() {
        valueMutex.unlock();
    }

    /**
     * User state (can have multiple key/ values)
     */

    UserState::UserState(const std::string &userIn) : user(userIn) {

    }

    UserState::~UserState() {
        // Delete contents of key value store
        for (const auto &iter: kvMap) {
            delete iter.second;
        }

        kvMap.clear();
    }

    StateKeyValue *UserState::getValue(const std::string &key, size_t size) {
        if (kvMap.count(key) == 0) {
            if (size == 0) {
                throw std::runtime_error("Must provide a size when getting a value that's not already present");
            }

            // Lock on editing local state registry
            FullLock fullLock(kvMapMutex);

            std::string actualKey = user + "_" + key;

            // Double check it still doesn't exist
            if (kvMap.count(key) == 0) {
                auto kv = new StateKeyValue(actualKey, size);

                kvMap.emplace(KVPair(key, kv));
            }
        }

        return kvMap[key];
    }

    void UserState::pushAll() {
        // Iterate through all key-values
        SharedLock sharedLock(kvMapMutex);

        for (const auto &kv : kvMap) {
            // Attempt to push partial updates
            kv.second->pushPartial();

            // Attempt to push partial updates
            kv.second->pushFull();

            // Attempt to clear (will be ignored if not relevant)
            kv.second->clear();
        }
    }

    /**
     * Global state (can hold many users' state)
     */

    State &getGlobalState() {
        static State s;
        return s;
    }

    State::State() {
        const util::SystemConfig &conf = util::getSystemConfig();
        pushInterval = conf.statePushInterval;
    }

    State::~State() {
        // Delete contents of user state map
        for (const auto &iter: userStateMap) {
            delete iter.second;
        }

        userStateMap.clear();
    }

    void State::pushLoop() {
        while (true) {
            // usleep takes microseconds
            usleep(1000 * pushInterval);

            this->pushAll();
        }
    }

    void State::pushAll() {
        // Run the sync for all users' state
        for (const auto &iter: userStateMap) {
            iter.second->pushAll();
        }
    }

    void State::forceClearAll() {
        for (const auto &iter: userStateMap) {
            delete iter.second;
        }

        userStateMap.clear();
    }

    StateKeyValue *State::getKV(const std::string &user, const std::string &key, size_t size) {
        UserState *us = this->getUserState(user);
        return us->getValue(key, size);
    }

    UserState *State::getUserState(const std::string &user) {
        if (userStateMap.count(user) == 0) {
            // Lock on editing user state registry
            FullLock fullLock(userStateMapMutex);

            // Double check it still doesn't exist
            if (userStateMap.count(user) == 0) {
                auto s = new UserState(user);

                userStateMap.emplace(UserStatePair(user, s));
            }
        }

        return userStateMap[user];
    }
}