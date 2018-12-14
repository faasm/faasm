#include "infra.h"

namespace infra {
    /**
     * Key/value
     */
    StateKeyValue::StateKeyValue(const std::string &keyIn, Redis *redisIn) : key(keyIn), redis(redisIn) {
        _isDirty = false;
        isInitialised = false;
    }

    void StateKeyValue::initialise() {
        // Unique lock on the whole value while initialising
        std::unique_lock<std::shared_mutex> lock(valueMutex);

        // Drop out if initialisation has happened since we acquired the lock
        if (isInitialised) {
            return;
        }

        // Load the whole thing from remote
        value = redis->get(key);

        isInitialised = true;
    }

    std::vector<uint8_t> StateKeyValue::get() {
        if (!isInitialised) {
            this->initialise();
        }

        // Shared lock for reads
        std::shared_lock<std::shared_mutex> lock(valueMutex);
        return value;
    }

    std::vector<uint8_t> StateKeyValue::getSegment(long offset, long length) {
        if (!isInitialised) {
            this->initialise();
        }

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
        _isDirty = true;
        isInitialised = true;
    }

    void StateKeyValue::setSegment(long offset, const std::vector<uint8_t> &data) {
        if (!isInitialised) {
            this->initialise();
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

    void StateKeyValue::sync() {
        // Get unique lock the value for syncing
        std::unique_lock<std::shared_mutex> valueLock(valueMutex);

        // If nothing dirty, ignore
        if (!_isDirty && dirtySegments.empty()) {
            return;
        }

        // If whole value is dirty, run the full update
        if (_isDirty) {
            redis->set(key, value);
            _isDirty = false;
            dirtySegments.clear();
            return;
        }

        // At this point we just have dirty segments, update those
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

    State::State() {
        redis = new Redis(STATE);
    }

    State::~State() {
        delete redis;

        // Delete contents of local state
        for (const auto &iter: local) {
            delete iter.second;
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
}