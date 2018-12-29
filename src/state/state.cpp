#include <algorithm>
#include <sys/mman.h>

#include "state.h"

namespace state {

    /**
     * Key/value
     */
    StateKeyValue::StateKeyValue(const std::string &keyIn, size_t sizeIn) : key(keyIn),
                                                                            valueSize(sizeIn) {

        // Work out size of required shared memory
        size_t nHostPages = util::getRequiredHostPages(valueSize);
        sharedMemSize = nHostPages * util::HOST_PAGE_SIZE;
        sharedMemory = nullptr;

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

        if (async && fullAsync) {
            // Never pull in full async mode
        } else if (async) {
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
            initialiseStorage();
        }

        // Read from the remote
        infra::Redis *redis = infra::Redis::getThreadState();
        redis->get(key, (uint8_t *) sharedMemory, valueSize);

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

    void StateKeyValue::preGet() {
        if (this->empty()) {
            throw std::runtime_error("Must pull before accessing state");
        }

        this->updateLastInteraction();
    }

    void StateKeyValue::get(uint8_t *buffer) {
        this->preGet();

        SharedLock lock(valueMutex);

        std::copy((uint8_t *) sharedMemory, ((uint8_t *) sharedMemory) + valueSize, buffer);
    }

    uint8_t *StateKeyValue::get() {
        this->preGet();

        SharedLock lock(valueMutex);

        return (uint8_t *) sharedMemory;
    }

    void StateKeyValue::getSegment(long offset, uint8_t *buffer, size_t length) {
        this->preGet();

        SharedLock lock(valueMutex);

        // Return just the required segment
        if ((offset + length) > valueSize) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

            logger->error("Out of bounds read at {} on {} with length {}", offset + length, key, valueSize);
            throw std::runtime_error("Out of bounds read");
        }

        auto bytePtr = (uint8_t *) sharedMemory;
        std::copy(bytePtr + offset, bytePtr + offset + length, buffer);
    }

    uint8_t *StateKeyValue::getSegment(long offset, long len) {
        this->preGet();

        SharedLock lock(valueMutex);

        return ((uint8_t *) sharedMemory) + offset;
    }

    void StateKeyValue::set(const uint8_t *buffer) {
        this->updateLastInteraction();

        // Unique lock for setting the whole value
        FullLock lock(valueMutex);

        if (sharedMemory == nullptr) {
            initialiseStorage();
        }

        // Copy data into shared region
        std::copy(buffer, buffer + valueSize, (uint8_t *) sharedMemory);

        isWholeValueDirty = true;
        _empty = false;
    }

    void StateKeyValue::setSegment(long offset, const uint8_t *buffer, size_t length) {
        this->updateLastInteraction();

        // Check we're in bounds
        size_t end = offset + length;
        if (end > valueSize) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->error("Trying to write segment finishing at {} (value length {})", end, valueSize);
            throw std::runtime_error("Attempting to set segment out of bounds");
        }

        // If empty, set to full size
        if (sharedMemory == nullptr) {
            FullLock lock(valueMutex);
            if (sharedMemory == nullptr) {
                initialiseStorage();
            }
        }

        // Check size
        if (offset + length > valueSize) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

            logger->error("Segment length {} at offset {} too big for size {}", length, offset, valueSize);
            throw std::runtime_error("Setting state segment too big for container");
        }

        // Copy data into shared region
        SharedLock lock(valueMutex);
        std::copy(buffer, buffer + length, ((uint8_t *) sharedMemory) + offset);

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

                // Set flag to say this is effectively new again
                _empty = true;
            }
        }
    }

    bool StateKeyValue::empty() {
        return _empty;
    }

    void StateKeyValue::mapSharedMemory(void *newAddr) {
        FullLock lock(valueMutex);
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Remap our existing shared memory onto this new region
        void *result = mremap(sharedMemory, 0, sharedMemSize, MREMAP_FIXED | MREMAP_MAYMOVE, newAddr);
        if (result == MAP_FAILED) {
            logger->error("Failed to map shared memory at {} with size {}. errno: {}",
                          sharedMemory, sharedMemSize, errno);

            throw std::runtime_error("Failed mapping shared memory");
        }

        if (newAddr != result) {
            logger->error("New mapped addr doesn't match required {} != {}",
                          newAddr, result);

            throw std::runtime_error("Misaligned shared memory mapping");
        }
    }

    void StateKeyValue::unmapSharedMemory(void *mappedAddr) {
        FullLock lock(valueMutex);
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if (!util::isPageAligned(mappedAddr)) {
            logger->error("Attempting to unmap non-page-aligned memory at {} for {}", mappedAddr, key);
            throw std::runtime_error("Unmapping misaligned shared memory");
        }

        // Unmap the current memory so it can be reused
        int result = munmap(mappedAddr, sharedMemSize);
        if (result == -1) {
            logger->error("Failed to unmap shared memory at {} with size {}. errno: {}", mappedAddr, sharedMemSize,
                          errno);

            throw std::runtime_error("Failed unmapping shared memory");
        }
    }

    void StateKeyValue::initialiseStorage() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Create shared memory region
        sharedMemory = mmap(nullptr, sharedMemSize, PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if (sharedMemory == MAP_FAILED) {
            logger->debug("Mmapping of storage size {} failed. errno: {}", sharedMemSize, errno);

            throw std::runtime_error("Failed mapping memory for KV");
        }

        logger->debug("Mmapped {} pages of shared storage for {}", sharedMemSize / util::HOST_PAGE_SIZE, key);

        // Set up dirty flags
        dirtyFlags.resize(valueSize);
    }

    void StateKeyValue::pushFull() {
        if (fullAsync) {
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
        redis->set(key, (uint8_t *) sharedMemory, valueSize);

        // Reset (as we're setting the full value, we've effectively pulled)
        util::Clock &clock = util::getGlobalClock();
        lastPull = clock.now();
        isWholeValueDirty = false;

        // Remove any dirty flags
        std::fill(dirtyFlags.begin(), dirtyFlags.end(), false);
    }

    void StateKeyValue::pushPartial() {
        if (fullAsync) {
            throw std::runtime_error("Shouldn't be pushing in full async mode.");
        }

        // Ignore if the whole value is dirty
        if (isWholeValueDirty) {
            return;
        }

        // Create copy of the dirty flags and clear the old version
        std::vector<bool> dirtyFlagsCopy(valueSize);
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
                    (uint8_t *) sharedMemory + offset,
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