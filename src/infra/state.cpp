#include "infra.h"

namespace infra {

    std::mutex existenceMutex;

    /**
     * Note, this class is always shared between threads so be very careful
     * with thread safety
     */
    StateSegment::StateSegment(long startIdxIn, long endIdxIn) : startIdx(startIdxIn), endIdx(endIdxIn) {

    }

    StateKeyValue::StateKeyValue(const std::string &keyIn, Redis *redisIn) : key(keyIn), redis(redisIn) {

    }

    void StateKeyValue::initialise() {
        std::lock_guard<std::mutex> lck(mutex);
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

    State::State() {
        redis = new Redis(STATE);
    }

    State::~State() {
        delete redis;
    }

    bool State::existsLocally(const std::string &key) {
        return local.count(key) > 0;
    }

    StateKeyValue* State::getKV(const std::string &key) {
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