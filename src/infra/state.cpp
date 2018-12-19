#include <prof/prof.h>
#include "infra.h"

namespace infra {
    typedef std::unique_lock<std::shared_mutex> FullLock;
    typedef std::shared_lock<std::shared_mutex> SharedLock;

    /**
     * Key/value
     */
    StateKeyValue::StateKeyValue(const std::string &keyIn) : key(keyIn), clock(util::getGlobalClock()) {
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

    void StateKeyValue::doRemoteRead() {
        // Read from the remote
        Redis *redis = infra::Redis::getThreadState();
        value = redis->get(key);

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
        isWholeValueDirty = true;
        isNew = false;
    }

    void StateKeyValue::setSegment(long offset, const std::vector<uint8_t> &data) {
        this->updateLastInteraction();

        // Resize vector if needed
        size_t max = offset + data.size();
        if (max > value.size()) {
            // Unique lock for changing value size
            FullLock lock(valueMutex);

            // Double check condition still holds
            if (max > value.size()) {
                // Do the resize
                value.resize(max);
            }
        }

        // Shared lock for writing segments (need to allow lock-free writing of multiple threads on same
        // state value)
        SharedLock lock(valueMutex);

        // Record that this segment is dirty
        dirtySegments.insert(Segment(offset, offset + data.size()));

        // Update the value itself
        std::copy(data.begin(), data.end(), value.begin() + offset);
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

        Redis *redis = infra::Redis::getThreadState();
        redis->set(key, value);

        // Reset (as we're setting the full value, we've effectively pulled)
        lastPull = clock.now();
        isWholeValueDirty = false;
        dirtySegments.clear();
    }

    SegmentSet StateKeyValue::mergeSegments(SegmentSet setIn) {
        SegmentSet mergedSet;

        if(setIn.size() < 2) {
            mergedSet = setIn;
            return mergedSet;
        }

        // Note: standard set sort order does fine here
        long count = 0;
        long currentStart = INT_MAX;
        long currentEnd = -INT_MAX;

        for(const auto p : setIn) {
            // On first loop, just set up
            if(count == 0) {
                currentStart = p.first;
                currentEnd = p.second;
                count++;
                continue;
            }

            if(p.first > currentEnd) {
                // If new segment is disjoint, write the last one and continue
                mergedSet.insert(Segment(currentStart, currentEnd));

                currentStart = p.first;
                currentEnd = p.second;
            }
            else {
                // Update current segment if not
                currentEnd = std::max(p.second, currentEnd);
            }

            // If on the last loop, make sure we've recorded the current range
            if(count == setIn.size() -1) {
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

            Redis *redis = infra::Redis::getThreadState();
            redis->setRange(
                    key,
                    segment.first,
                    dataSegment
            );
        }
    }

    /**
     * State (can have multiple key/ values)
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
        // Delete contents of local state
        for (const auto &iter: local) {
            delete iter.second;
        }
    }

    void State::pushLoop() {
        while (true) {
            // usleep takes microseconds
            usleep(1000 * pushInterval);

            // Run the sync
            this->pushAll();
        }
    }

    StateKeyValue *State::getKV(const std::string &key) {
        if (local.count(key) == 0) {
            // Lock on editing local state registry
            FullLock fullLock(localMutex);

            // Double check it still doesn't exist
            if (local.count(key) == 0) {
                auto kv = new StateKeyValue(key);

                local.emplace(KVPair(key, kv));
            }
        }

        return local[key];
    }

    void State::pushAll() {
        // Iterate through all key-values
        SharedLock sharedLock(localMutex);

        for (const auto &kv : local) {
            // Attempt to push partial updates
            kv.second->pushPartial();

            // Attempt to push partial updates
            kv.second->pushFull();

            // Attempt to clear (will be ignored if not relevant)
            kv.second->clear();
        }
    }
}