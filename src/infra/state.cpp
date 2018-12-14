#include "infra.h"

namespace infra {
    /**
     * Key/value
     */
    StateKeyValue::StateKeyValue(const std::string &keyIn, Redis *redisIn) : key(keyIn), redis(redisIn) {
        isWholeValueDirty = false;
        isNew = true;

        // Gets over the stale threshold trigger a pull from remote
        staleThreshold = std::stol(util::getEnvVar("STALE_THRESHOLD", "1000"));

        // State over the clear threshold is removed from local
        clearThreshold = std::stol(util::getEnvVar("CLEAR_THRESHOLD", "10000"));
    }

    void StateKeyValue::pull() {
        // Check if new (one-off initialisation)
        if (isNew) {
            // Unique lock on the whole value while loading
            std::unique_lock<std::shared_mutex> lock(valueMutex);

            // Double check assumption
            if (isNew) {
                value = redis->get(key);
                lastRemoteRead = std::chrono::steady_clock::now();
                isNew = false;
            }
        }

        // Check staleness
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        long dt = this->getAge(now);

        // If stale, try to update from remote
        if (dt > staleThreshold) {
            // Unique lock on the whole value while loading
            std::unique_lock<std::shared_mutex> lock(valueMutex);

            // Double check staleness
            long dt2 = this->getAge(now);
            if (dt2 > staleThreshold) {
                // Read from the remote
                value = redis->get(key);
                lastRemoteRead = std::chrono::steady_clock::now();
            }
        }

    }

    long StateKeyValue::getAge(const std::chrono::steady_clock::time_point &now) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - lastRemoteRead).count();
    }


    std::vector<uint8_t> StateKeyValue::get() {
        this->pull();

        // Shared lock for reads
        std::shared_lock<std::shared_mutex> lock(valueMutex);
        return value;
    }

    std::vector<uint8_t> StateKeyValue::getSegment(long offset, long length) {
        this->pull();

        // Shared lock for reads
        std::shared_lock<std::shared_mutex> lock(valueMutex);

        // Return just the required segment
        std::vector<uint8_t> segment(value.begin() + offset, value.begin() + offset + length);
        return segment;
    }

    void StateKeyValue::set(const std::vector<uint8_t> &data) {
        // Unique lock for setting the whole value
        std::unique_lock<std::shared_mutex> lock(valueMutex);

        this->value = data;
        isWholeValueDirty = true;
        isNew = false;
    }

    void StateKeyValue::setSegment(long offset, const std::vector<uint8_t> &data) {
        size_t max = offset + data.size();

        // Resize vector if needed
        if (max > value.size()) {
            // Unique lock for changing value size
            valueMutex.lock();

            // Double check condition still holds
            if (max > value.size()) {
                // Do the resize
                std::unique_lock<std::shared_mutex> resizeLock(valueMutex);
                value.resize(max);
            }

            valueMutex.unlock();
        }

        // Shared lock for writing segments (need to allow lock-free writing of multiple threads on same
        // state value)
        std::shared_lock<std::shared_mutex> valueLock(valueMutex);

        // TODO - is std::set insert thread-safe in this context?
        // Record that this segment is dirty
        dirtySegments.insert(std::pair<long, long>(offset, data.size()));

        // Update the value itself
        std::copy(data.begin(), data.end(), value.begin() + offset);
    }

    void StateKeyValue::clear() {
        // Check age since last remote read
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        long dt = this->getAge(now);

        // If over clear threshold, remove
        if (dt > clearThreshold) {
            // Unique lock on the whole value while loading
            std::unique_lock<std::shared_mutex> lock(valueMutex);

            long dt2 = this->getAge(now);
            // Double check still over the threshold
            if (dt2 > clearThreshold) {
                // Make sure everything has been pushed
                this->push();

                // Totally remove the value
                this->value.clear();
                isNew = true;
            }
        }
    }

    void StateKeyValue::push() {
        // TODO - require lock for this check?
        // Skip the sync if nothing dirty
        if (!isWholeValueDirty && dirtySegments.empty()) {
            return;
        }

        // Get unique lock the value for syncing
        std::unique_lock<std::shared_mutex> valueLock(valueMutex);

        // If whole value is dirty, run the full update and drop out
        if (isWholeValueDirty) {
            redis->set(key, value);
            isWholeValueDirty = false;
            dirtySegments.clear();
            return;
        }

        // If segments to write, write those
        for (const auto segment : dirtySegments) {
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

        // Reset and read back the full value
        dirtySegments.clear();
        value = redis->get(key);
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

    bool State::existsLocally(const std::string &key) {
        return local.count(key) > 0;
    }

    StateKeyValue *State::getKV(const std::string &key) {
        // Synchronisation here is important:
        // - If key exists locally, that's fine, return a reference
        // - If not, there's a race to create the object
        bool exists = this->existsLocally(key);
        if (!exists) {
            localMutex.lock();

            // Double check it still doesn't exist
            if (!this->existsLocally(key)) {
                auto kv = new StateKeyValue(key, redis);

                local.emplace(KVPair(key, kv));
            }

            localMutex.unlock();
        }

        return local[key];
    }

    void State::pushAll() {
        // Iterate through all key-values
        for (const auto &kv : local) {
            // Attempt to push (will be ignored if not relevant)
            kv.second->push();

            // Attempt to clear (will be ignored if not relevant)
            kv.second->clear();
        }
    }
}