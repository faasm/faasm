#include "infra.h"

namespace infra {

    std::string getSegmentKey(long offset, long length) {
        return std::to_string(offset) + "_" + std::to_string(length);
    }

    StateSegment::StateSegment(const std::string &keyIn, Redis *redisIn, long offsetIn, long lengthIn) :
            _string(getSegmentKey(offsetIn, lengthIn)), key(keyIn), redis(redisIn), offset(offsetIn), length(lengthIn) {

    }

    const std::string StateSegment::string() {
        return _string;
    }

    StateKeyValue::StateKeyValue(const std::string &keyIn, Redis *redisIn) : key(keyIn), redis(redisIn) {
        _isDirty = false;
        isInitialised = false;
    }

    StateSegment *StateKeyValue::getSegment(long offset, long length) {
        std::string segmentStr = getSegmentKey(offset, length);
        bool exists = this->segmentExistsLocally(offset, length);
        if (!exists) {
            segmentMutex.lock();
            if (!this->segmentExistsLocally(offset, length)) {
                auto segment = new StateSegment(key, redis, offset, length);

                segments.emplace(SegmentPair(segment->string(), segment));
            }
            segmentMutex.unlock();
        }

        return segments[segmentStr];
    }

    bool StateKeyValue::segmentExistsLocally(long offset, long length) {
        return segments.count(getSegmentKey(offset, length)) > 0;
    }

    void StateKeyValue::initialise() {
        // Lock on the whole value
        std::lock_guard<std::mutex> lck(mtx);

        if (!isInitialised) {
            // Load the whole thing from remote
            value = redis->get(key);

            isInitialised = true;
        }
    }

    std::vector<uint8_t> StateKeyValue::get() {
        if (!isInitialised) {
            this->initialise();
        }

        return value;
    }

    std::vector<uint8_t> StateKeyValue::getPartial(long offset, long length) {
        if (!segmentExistsLocally(offset, length)) {
            this->initialiseSegment();
        }

        std::vector<uint8_t> partial(value.begin() + offset, value.begin() + offset + length);
        return partial;
    }

    void StateKeyValue::set(const std::vector<uint8_t> &data) {
        // Lock on the whole value
        std::lock_guard<std::mutex> lck(mtx);

        this->value = data;
        _isDirty = true;
    }

    void StateKeyValue::setPartial(long offset, const std::vector<uint8_t> &data) {

    }

    bool StateKeyValue::isDirty() {
        return _isDirty;
    }

    void StateKeyValue::sync() {
        // Lock and run update
        std::lock_guard<std::mutex> lck(mtx);

        if (_isDirty) {
            redis->set(key, value);
            _isDirty = false;
        }
    }

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