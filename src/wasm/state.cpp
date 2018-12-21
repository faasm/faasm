#include <prof/prof.h>

#include "wasm.h"

namespace wasm {
    typedef std::unique_lock<std::shared_mutex> FullLock;
    typedef std::shared_lock<std::shared_mutex> SharedLock;

    /**
     * Shared memory segment
     */
    StateMemorySegment::StateMemorySegment(Uptr pageIn, uint8_t *ptrIn, size_t lengthIn) : page(pageIn), ptr(ptrIn),
                                                                                           length(lengthIn) {

    }

    /**
     * Shared memory
     */
    StateMemory::StateMemory(const std::string &userIn) : user(userIn) {
        compartment = Runtime::createCompartment();

        // Prepare memory, each user can have up to 1000
        IR::MemoryType memoryType(true, {0, 1000});

        // Create a shared memory for this user
        wavmMemory = Runtime::createMemory(compartment, memoryType, user + "_shared");
        Runtime::growMemory(wavmMemory, 5);

        nextPage = 0;
    }

    StateMemory::~StateMemory() {
        wavmMemory = nullptr;

        Runtime::tryCollectCompartment(std::move(compartment));
    };

    uint8_t *StateMemory::getPointer(size_t length) {
        const std::shared_ptr<spdlog::logger> logger = util::getLogger();

        // Need to lock the whole memory to make changes
        FullLock lock(memMutex);

        const Uptr pagesToAdd = getNumberOfPagesForBytes(length);
        const Uptr thisPage = nextPage;
        nextPage += pagesToAdd;

        // See if we need to grow
        const Uptr currentPageCount = getMemoryNumPages(wavmMemory);
        Uptr maxPages = getMemoryMaxPages(wavmMemory);
        if (nextPage > maxPages) {
            logger->error("Allocating {} pages of shared memory when max is {}", nextPage, maxPages);
            throw std::runtime_error("Attempting to allocate more than max pages of shared memory.");
        }

        // Grow memory if required
        if (nextPage > currentPageCount) {
            Uptr expansion = nextPage - currentPageCount;
            growMemory(wavmMemory, expansion);
        }

        // Return pointer to memory
        U8 *data = Runtime::memoryArrayPtr<U8>(wavmMemory, thisPage, length);

        // Record the use of this segment
        segments.emplace_back(StateMemorySegment(thisPage, data, length));

        return data;
    }

    void StateMemory::releaseSegment(uint8_t *ptr) {
        // Lock the memory to make changes
        FullLock lock(memMutex);

        for (auto s : segments) {
            if (s.ptr == ptr) {
                s.inUse = false;
            }
        }
    }

    /**
     * Key/value
     */
    StateKeyValue::StateKeyValue(const std::string &keyIn, StateMemory *sharedMemoryIn) : key(keyIn),
                                                                                          clock(util::getGlobalClock()),
                                                                                          sharedMemory(sharedMemoryIn) {
        isWholeValueDirty = false;
        isNew = true;

        // Gets over the stale threshold trigger a pull from remote
        const util::SystemConfig &conf = util::getSystemConfig();
        staleThreshold = conf.stateStaleThreshold;

        // State over the clear threshold is removed from local
        idleThreshold = conf.stateClearThreshold;
    }

    void StateKeyValue::pull() {
        // Check if new (one-off initialisation)
        if (isNew) {
            // Unique lock on the whole value while loading
            FullLock lock(valueMutex);

            // Double check assumption
            if (isNew) {
                const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
                logger->debug("Initialising state for {}", key);

                doRemoteRead();
                isNew = false;
                return;
            }
        }

        // Check staleness
        const util::TimePoint now = clock.now();

        // If stale, try to update from remote
        if (this->isStale(now)) {
            // Unique lock on the whole value while loading
            FullLock lock(valueMutex);

            // Double check staleness
            if (this->isStale(now)) {
                const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
                logger->debug("Refreshing stale state for {}", key);

                doRemoteRead();
            }
        }
    }

    void StateKeyValue::copySegmentToSharedMem(long start, long end) {
        // Copy data into new shared region
        std::copy(value.begin() + start, value.begin() + end, sharedMemoryPtr + start);
    }

    void StateKeyValue::copyValueToSharedMem() {
        if (!value.empty()) {
            // Set up the shared memory region
            if (sharedMemoryPtr == nullptr) {
                sharedMemoryPtr = sharedMemory->getPointer(value.size());
            }

            // Copy data into new shared region
            std::copy(value.begin(), value.end(), sharedMemoryPtr);
        }
    }

    void StateKeyValue::doRemoteRead() {
        // Read from the remote
        infra::Redis *redis = infra::Redis::getThreadState();
        value = redis->get(key);

        // Copy into shared memory
        this->copyValueToSharedMem();

        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        logger->debug("Value for {} size {} after read", key, value.size());

        const util::TimePoint now = clock.now();
        lastPull = now;

        this->updateLastInteraction();
    }

    void StateKeyValue::updateLastInteraction() {
        const util::TimePoint now = clock.now();
        lastInteraction = now;
    }

    bool StateKeyValue::isStale(const util::TimePoint &now) {
        long age = clock.timeDiff(now, lastPull);
        return age > staleThreshold;
    }

    bool StateKeyValue::isIdle(const util::TimePoint &now) {
        long idleTime = clock.timeDiff(now, lastInteraction);
        return idleTime > idleThreshold;
    }

    std::vector<uint8_t> StateKeyValue::get() {
        this->updateLastInteraction();

        this->pull();

        // Shared lock for full reads
        SharedLock lock(valueMutex);

        return value;
    }

    std::vector<uint8_t> StateKeyValue::getSegment(long offset, long length) {
        this->updateLastInteraction();

        this->pull();

        // Shared lock for partial reads
        SharedLock lock(valueMutex);

        // Return just the required segment
        if ((offset + length) > value.size()) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

            logger->error("Out of bounds read at {} on {} with length {}", offset + length, key, value.size());
            throw std::runtime_error("Out of bounds read");
        }

        std::vector<uint8_t> segment(value.begin() + offset, value.begin() + offset + length);
        return segment;
    }

    void StateKeyValue::set(const std::vector<uint8_t> &data) {
        this->updateLastInteraction();

        // Unique lock for setting the whole value
        FullLock lock(valueMutex);

        this->value = data;
        this->copyValueToSharedMem();
        isWholeValueDirty = true;
        isNew = false;
    }

    void StateKeyValue::setSegment(long offset, const std::vector<uint8_t> &data) {
        this->updateLastInteraction();

        // Get the value first
        this->pull();

        // Check we're in bounds
        size_t end = offset + data.size();
        if (end > value.size()) {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
            logger->error("Trying to write segment finishing at {} (value length {})", end, value.size());
            throw std::runtime_error("Attempting to set segment out of bounds");
        }

        // Shared lock for writing segments (need to allow lock-free writing of multiple threads on same
        // state value)
        SharedLock lock(valueMutex);

        // Record that this segment is dirty
        dirtySegments.insert(Segment(offset, end));

        // Update the value itself
        std::copy(data.begin(), data.end(), value.begin() + offset);

        // Update shared memory
        this->copySegmentToSharedMem(offset, end);
    }

    void StateKeyValue::clear() {
        // Check age since last interaction
        util::Clock &c = util::getGlobalClock();
        util::TimePoint now = c.now();

        // If over clear threshold, remove
        if (this->isIdle(now) && !value.empty()) {
            // Unique lock on the whole value while clearing
            FullLock lock(valueMutex);

            // Double check still over the threshold
            if (this->isIdle(now)) {
                const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
                logger->debug("Clearing unused value {}", key);

                // Totally remove the value
                this->value.clear();

                // Release shared state
                sharedMemory->releaseSegment(sharedMemoryPtr);

                // Set flag to say this is effectively new again
                isNew = true;
            }
        }
    }

    long StateKeyValue::getLocalValueSize() {
        return value.size();
    }

    void StateKeyValue::pushFull() {
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
        redis->set(key, value);

        // Reset (as we're setting the full value, we've effectively pulled)
        lastPull = clock.now();
        isWholeValueDirty = false;
        dirtySegments.clear();
    }

    SegmentSet StateKeyValue::mergeSegments(SegmentSet setIn) {
        SegmentSet mergedSet;

        if (setIn.size() < 2) {
            mergedSet = setIn;
            return mergedSet;
        }

        // Note: standard set sort order does fine here
        long count = 0;
        long currentStart = INT_MAX;
        long currentEnd = -INT_MAX;

        for (const auto p : setIn) {
            // On first loop, just set up
            if (count == 0) {
                currentStart = p.first;
                currentEnd = p.second;
                count++;
                continue;
            }

            if (p.first > currentEnd) {
                // If new segment is disjoint, write the last one and continue
                mergedSet.insert(Segment(currentStart, currentEnd));

                currentStart = p.first;
                currentEnd = p.second;
            } else {
                // Update current segment if not
                currentEnd = std::max(p.second, currentEnd);
            }

            // If on the last loop, make sure we've recorded the current range
            if (count == setIn.size() - 1) {
                mergedSet.insert(Segment(currentStart, currentEnd));
            }

            count++;
        }

        return mergedSet;
    }

    void StateKeyValue::pushPartial() {
        // Ignore if the whole value is dirty
        if (isWholeValueDirty) {
            return;
        }

        // Create copy of the dirty segments and clear the old version
        SegmentSet dirtySegmentsCopy;
        {
            FullLock fullLock(valueMutex);
            dirtySegmentsCopy = dirtySegments;
            dirtySegments.clear();
        }

        // Merge segments that are next to each other to save on writes
        SegmentSet segmentsToMerge = this->mergeSegments(dirtySegmentsCopy);

        // Shared lock for writing segments
        SharedLock sharedLock(valueMutex);

        // Write the dirty segments
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        for (const auto segment : segmentsToMerge) {
            logger->debug("Pushing partial value for {} ({} - {})", key, segment.first, segment.second);

            std::vector<uint8_t> dataSegment(
                    value.begin() + segment.first,
                    value.begin() + segment.second
            );

            infra::Redis *redis = infra::Redis::getThreadState();
            redis->setRange(
                    key,
                    segment.first,
                    dataSegment
            );
        }
    }

    /**
     * User state (can have multiple key/ values)
     */

    UserState::UserState(const std::string &userIn) : user(userIn) {
        memory = new StateMemory(userIn);
    }

    UserState::~UserState() {
        // Delete contents of key value store
        for (const auto &iter: kvMap) {
            delete iter.second;
        }

        // Delete memory
        delete memory;
    }

    StateKeyValue *UserState::getValue(const std::string &key) {
        if (kvMap.count(key) == 0) {
            // Lock on editing local state registry
            FullLock fullLock(kvMapMutex);

            std::string actualKey = user + "_" + key;

            // Double check it still doesn't exist
            if (kvMap.count(key) == 0) {
                auto kv = new StateKeyValue(actualKey, memory);

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

    StateKeyValue *State::getKV(const std::string &user, const std::string &key) {
        UserState *us = this->getUserState(user);
        return us->getValue(key);
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