#include <algorithm>
#include <sys/mman.h>
#include <unistd.h>

#include "state.h"

using namespace util;

namespace state {

    /**
     * Key/value
     */
    StateKeyValue::StateKeyValue(const std::string &keyIn, size_t sizeIn) : key(keyIn),
                                                                            valueSize(sizeIn) {

        // Work out size of required shared memory
        size_t nHostPages = getRequiredHostPages(valueSize);
        sharedMemSize = nHostPages * HOST_PAGE_SIZE;
        sharedMemory = nullptr;

        isWholeValueDirty = false;
        isPartiallyDirty = false;
        _empty = true;

        // Gets over the stale threshold trigger a pull from remote
        const SystemConfig &conf = getSystemConfig();
        staleThreshold = conf.stateStaleThreshold;

        // State over the clear threshold is removed from local
        idleThreshold = conf.stateClearThreshold;

        // Whether to run in fully async mode
        fullAsync = conf.fullAsync > 0;
    }

    void StateKeyValue::pull(bool async) {
        this->updateLastInteraction();
        const std::shared_ptr<spdlog::logger> &logger = getLogger();

        // Check if new (one-off initialisation)
        if (_empty) {
            // Unique lock on the whole value while loading
            FullLock lock(valueMutex);

            // Double check assumption
            if (_empty) {
                logger->debug("Initialising state for {}", key);

                doRemoteRead();
                _empty = false;
            }

            return;
        }

        if (async && fullAsync) {
            // Never pull in full async mode
            logger->debug("Ignoring pull in full async mode for {}", key);

        } else if (async) {
            // Check staleness
            Clock &clock = getGlobalClock();
            const TimePoint now = clock.now();

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
        infra::Redis &redis = infra::Redis::getState();

        const std::shared_ptr<spdlog::logger> &logger = getLogger();
        logger->debug("Reading from remote for {}", key);

        redis.get(key, static_cast<uint8_t *>(sharedMemory), valueSize);

        Clock &clock = getGlobalClock();
        const TimePoint now = clock.now();
        lastPull = now;

        this->updateLastInteraction();
    }

    void StateKeyValue::updateLastInteraction() {
        Clock &clock = getGlobalClock();
        const TimePoint now = clock.now();
        lastInteraction = now;
    }

    bool StateKeyValue::isStale(const TimePoint &now) {
        Clock &clock = getGlobalClock();
        long age = clock.timeDiff(now, lastPull);
        return age > staleThreshold;
    }

    bool StateKeyValue::isIdle(const TimePoint &now) {
        Clock &clock = getGlobalClock();
        long idleTime = clock.timeDiff(now, lastInteraction);
        return idleTime > idleThreshold;
    }

    long StateKeyValue::waitOnRemoteLock() {
        const std::shared_ptr<spdlog::logger> &logger = getLogger();

        infra::Redis &redis = infra::Redis::getState();

        long remoteLockId = redis.acquireLock(key, remoteLockTimeout);
        int retryCount = 0;
        while (remoteLockId <= 0) {
            logger->debug("Waiting on remote lock for {} (loop {})", key, retryCount);

            if (retryCount >= remoteLockMaxRetries) {
                logger->error("Timed out waiting for lock on {}", key);
                break;
            }

            // usleep in microseconds
            usleep(remoteLockWaitTime * 1000);

            remoteLockId = redis.acquireLock(key, remoteLockTimeout);
            retryCount++;
        }

        return remoteLockId;
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

        auto bytePtr = static_cast<uint8_t *>(sharedMemory);
        std::copy(bytePtr, bytePtr + valueSize, buffer);
    }

    uint8_t *StateKeyValue::get() {
        this->preGet();

        SharedLock lock(valueMutex);

        return static_cast<uint8_t *>(sharedMemory);
    }

    void StateKeyValue::getSegment(long offset, uint8_t *buffer, size_t length) {
        this->preGet();

        SharedLock lock(valueMutex);

        // Return just the required segment
        if ((offset + length) > valueSize) {
            const std::shared_ptr<spdlog::logger> &logger = getLogger();

            logger->error("Out of bounds read at {} on {} with length {}", offset + length, key, valueSize);
            throw std::runtime_error("Out of bounds read");
        }

        auto bytePtr = static_cast<uint8_t *>(sharedMemory);
        std::copy(bytePtr + offset, bytePtr + offset + length, buffer);
    }

    uint8_t *StateKeyValue::getSegment(long offset, long len) {
        this->preGet();

        SharedLock lock(valueMutex);

        uint8_t *segmentPtr = static_cast<uint8_t *>(sharedMemory) + offset;
        return segmentPtr;
    }

    void StateKeyValue::set(const uint8_t *buffer) {
        this->updateLastInteraction();

        // Unique lock for setting the whole value
        FullLock lock(valueMutex);

        if (sharedMemory == nullptr) {
            initialiseStorage();
        }

        // Copy data into shared region
        std::copy(buffer, buffer + valueSize, static_cast<uint8_t *>(sharedMemory));

        isWholeValueDirty = true;
        _empty = false;
    }

    void StateKeyValue::setSegment(long offset, const uint8_t *buffer, size_t length) {
        this->updateLastInteraction();

        const std::shared_ptr<spdlog::logger> &logger = getLogger();

        // Check we're in bounds
        size_t end = offset + length;
        if (end > valueSize) {
            logger->error("Trying to write segment finishing at {} (value length {})", end, valueSize);
            throw std::runtime_error("Attempting to set segment out of bounds");
        }

        // If empty, set to full size
        if (sharedMemory == nullptr) {
            FullLock lock(valueMutex);

            if (sharedMemory == nullptr) {
                initialiseStorage();
                _empty = false;
            }
        }

        // Check size
        if (offset + length > valueSize) {
            logger->error("Segment length {} at offset {} too big for size {}", length, offset, valueSize);
            throw std::runtime_error("Setting state segment too big for container");
        }

        // Copy data into shared region
        {
            SharedLock lock(valueMutex);
            auto bytePtr = static_cast<uint8_t *>(sharedMemory);
            std::copy(buffer, buffer + length, bytePtr + offset);
        }

        this->flagSegmentDirty(offset, length);
    }

    void StateKeyValue::flagFullValueDirty() {
        isWholeValueDirty = true;
    }

    void StateKeyValue::flagSegmentDirty(long offset, long len) {
        SharedLock lock(valueMutex);
        isPartiallyDirty = true;
        std::fill(dirtyFlags.begin() + offset, dirtyFlags.begin() + offset + len, true);
    }

    void StateKeyValue::clear() {
        // Check age since last interaction
        Clock &c = getGlobalClock();
        TimePoint now = c.now();

        // If over clear threshold, remove
        if (this->isIdle(now) && !_empty) {
            // Unique lock on the whole value while clearing
            FullLock lock(valueMutex);

            // Double check still over the threshold
            if (this->isIdle(now)) {
                const std::shared_ptr<spdlog::logger> &logger = getLogger();
                logger->debug("Clearing unused value {}", key);

                // Set flag to say this is effectively new again
                _empty = true;
            }
        }
    }

    bool StateKeyValue::empty() {
        return _empty;
    }

    size_t StateKeyValue::size() {
        return valueSize;
    }

    void StateKeyValue::mapSharedMemory(void *newAddr) {
        const std::shared_ptr<spdlog::logger> &logger = getLogger();

        if (!isPageAligned(newAddr)) {
            logger->error("Attempting to map non-page-aligned memory at {} for {}", newAddr, key);
            throw std::runtime_error("Mapping misaligned shared memory");
        }

        FullLock lock(valueMutex);

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
        const std::shared_ptr<spdlog::logger> &logger = getLogger();

        if (!isPageAligned(mappedAddr)) {
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
        const std::shared_ptr<spdlog::logger> &logger = getLogger();

        // Create shared memory region
        sharedMemory = mmap(nullptr, sharedMemSize, PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if (sharedMemory == MAP_FAILED) {
            logger->debug("Mmapping of storage size {} failed. errno: {}", sharedMemSize, errno);

            throw std::runtime_error("Failed mapping memory for KV");
        }

        logger->debug("Mmapped {} pages of shared storage for {}", sharedMemSize / HOST_PAGE_SIZE, key);

        // Set up dirty flags
        dirtyFlags.resize(valueSize);
    }

    void StateKeyValue::pushFull() {
        if (fullAsync) {
            throw std::runtime_error("Shouldn't be pushing in full async mode.");
        }

        // Ignore if not dirty
        if (!isWholeValueDirty) {
            return;
        }

        const std::shared_ptr<spdlog::logger> &logger = getLogger();

        // Get full lock for complete push
        FullLock fullLock(valueMutex);

        // Double check condition
        if (!isWholeValueDirty) {
            return;
        }

        logger->debug("Pushing whole value for {}", key);

        infra::Redis &redis = infra::Redis::getState();
        redis.set(key, static_cast<uint8_t *>(sharedMemory), valueSize);

        // Reset (as we're setting the full value, we've effectively pulled)
        Clock &clock = getGlobalClock();
        lastPull = clock.now();
        isWholeValueDirty = false;

        // Remove any dirty flags
        isPartiallyDirty = false;
        std::fill(dirtyFlags.begin(), dirtyFlags.end(), false);
    }

    void StateKeyValue::pushPartial() {
        if (fullAsync) {
            throw std::runtime_error("Shouldn't be pushing in full async mode.");
        }

        // Ignore if the whole value is dirty or not partially dirty
        if (isWholeValueDirty || !isPartiallyDirty) {
            return;
        }

        const std::shared_ptr<spdlog::logger> &logger = getLogger();

        // Attempt to lock the value remotely
        infra::Redis &redis = infra::Redis::getState();
        long remoteLockId = this->waitOnRemoteLock();

        // If we don't get remote lock, just skip this push and wait for next one
        if (remoteLockId <= 0) {
            logger->debug("Failed to acquire remote lock for {}", key);
            return;
        }

        logger->debug("Got remote lock for {} with id {}", key, remoteLockId);

        // TODO Potential locking issues here.
        // If we get the remote lock, then have to wait a long time for the local
        // value lock, the remote one may have expired by the time we get to updating
        // the value, and we end up clashing.

        // Create copy of the dirty flags and clear the old version
        std::vector<bool> dirtyFlagsCopy(valueSize);
        {
            FullLock lock(valueMutex);

            // Double check condition
            if (isWholeValueDirty || !isPartiallyDirty) {
                return;
            }

            std::copy(dirtyFlags.begin(), dirtyFlags.end(), dirtyFlagsCopy.begin());

            // Reset dirty flags
            isPartiallyDirty = false;
            std::fill(dirtyFlags.begin(), dirtyFlags.end(), false);
        }

        // Don't need any more local locking here
        auto tempBuff = new uint8_t[valueSize];
        redis.get(key, tempBuff, valueSize);

        logger->debug("Pushing partial state for {}", key);

        // Go through all dirty flags and update value

        // Find first true flag
        auto start = std::find(dirtyFlagsCopy.begin(), dirtyFlagsCopy.end(), true);

        // While we still have more segments
        while (start != dirtyFlagsCopy.end()) {
            // Find next false
            auto end = std::find(start + 1, dirtyFlagsCopy.end(), false);

            // Our indices are inclusive here
            long size = end - start;
            long offset = start - dirtyFlagsCopy.begin();
            uint8_t *ptr = static_cast<uint8_t *>(sharedMemory) + offset;
            std::copy(ptr, ptr + size, tempBuff + offset);

            // Find next true
            start = std::find(end + 1, dirtyFlagsCopy.end(), true);
        }

        // Set whole value back again
        redis.set(key, tempBuff, valueSize);

        // Release remote lock
        redis.releaseLock(key, remoteLockId);
        logger->debug("Released remote lock for {} with id {}", key, remoteLockId);
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
        const SystemConfig &conf = getSystemConfig();
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