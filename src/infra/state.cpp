#include "infra.h"

namespace infra {
    typedef std::unique_lock<std::shared_mutex> FullLock;
    typedef std::shared_lock<std::shared_mutex> SharedLock;

    /**
     * Key/value
     */
    StateKeyValue::StateKeyValue(const std::string &keyIn, Redis *redisIn) : key(keyIn), redis(redisIn) {
        isWholeValueDirty = false;
        isNew = true;

        // Gets over the stale threshold trigger a pull from remote
        staleThreshold = std::stol(util::getEnvVar("STALE_THRESHOLD", "1000"));

        // State over the clear threshold is removed from local
        clearThreshold = std::stol(util::getEnvVar("CLEAR_THRESHOLD", "30000"));
    }

    void StateKeyValue::pull() {
        // Check if new (one-off initialisation)
        if (isNew) {
            // Unique lock on the whole value while loading
            FullLock lock(valueMutex);

            // Double check assumption
            if (isNew) {
                doRemoteRead();
                isNew = false;
            }
        }

        // Check staleness
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        long dt = this->getAge(now);

        // If stale, try to update from remote
        if (dt > staleThreshold) {
            // Unique lock on the whole value while loading
            FullLock lock(valueMutex);

            // Double check staleness
            long dt2 = this->getAge(now);
            if (dt2 > staleThreshold) {
                doRemoteRead();
            }
        }
    }

    void StateKeyValue::doRemoteRead() {
        // Read from the remote
        value = redis->get(key);

        const std::chrono::time_point now = std::chrono::steady_clock::now();
        lastPull = now;

        this->updateLastInteraction();
    }

    void StateKeyValue::updateLastInteraction() {
        const std::chrono::time_point now = std::chrono::steady_clock::now();
        lastInteraction = now;
    }

    long StateKeyValue::getAge(const std::chrono::steady_clock::time_point &now) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - lastPull).count();
    }

    long StateKeyValue::getIdleTime(const std::chrono::steady_clock::time_point &now) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - lastInteraction).count();
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
                std::unique_lock<std::shared_mutex> resizeLock(valueMutex);
                value.resize(max);
            }
        }

        // Shared lock for writing segments (need to allow lock-free writing of multiple threads on same
        // state value)
        SharedLock lock(valueMutex);

        // TODO - is std::set insert thread-safe in this context?
        // Record that this segment is dirty
        dirtySegments.insert(std::pair<long, long>(offset, data.size()));

        // Update the value itself
        std::copy(data.begin(), data.end(), value.begin() + offset);
    }

    void StateKeyValue::clear() {
        // Check age since last interaction
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        long dt = this->getIdleTime(now);

        // If over clear threshold, remove
        if (dt > clearThreshold) {
            // Unique lock on the whole value while clearing
            std::unique_lock<std::shared_mutex> lock(valueMutex);

            long dt2 = this->getIdleTime(now);

            // Double check still over the threshold
            if (dt2 > clearThreshold) {
                // Totally remove the value
                this->value.clear();
                isNew = true;
            }
        }
    }

    void StateKeyValue::push() {
        // Skip the push if nothing dirty
        if (!isWholeValueDirty && dirtySegments.empty()) {
            return;
        }

        // If whole value is dirty, run the full update and drop out
        if (isWholeValueDirty) {
            // Get full lock for complete write
            FullLock fullLock(valueMutex);

            redis->set(key, value);
            isWholeValueDirty = false;
            dirtySegments.clear();
            return;
        }

        // Handle partial push
        this->pushPartial();
    }

    void StateKeyValue::pushPartial() {
        // Create copy of the dirty segments and clear the old version
        std::set<std::pair<long, long>> dirtySegmentsCopy;
        {
            FullLock fullLock(valueMutex);
            dirtySegmentsCopy = dirtySegments;
            dirtySegments.clear();
        }

        // Write the dirty segments
        SharedLock sharedLock(valueMutex);
        for (const auto segment : dirtySegmentsCopy) {
            std::vector<uint8_t> dataSegment(
                    value.begin() + segment.first,
                    value.begin() + segment.first + segment.second
            );

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
        redis = new Redis(STATE);

        syncInterval = std::stol(util::getEnvVar("SYNC_INTERVAL", "50"));
    }

    State::~State() {
        delete redis;

        // Delete contents of local state
        for (const auto &iter: local) {
            delete iter.second;
        }
    }

    void State::pushLoop() {
        while (true) {
            usleep(1000 * syncInterval);

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
                auto kv = new StateKeyValue(key, redis);

                local.emplace(KVPair(key, kv));
            }
        }

        return local[key];
    }

    void State::pushAll() {
        // Iterate through all key-values
        SharedLock sharedLock(localMutex);

        for (const auto &kv : local) {
            // Attempt to push (will be ignored if not relevant)
            kv.second->push();

            // Attempt to clear (will be ignored if not relevant)
            kv.second->clear();
        }
    }
}