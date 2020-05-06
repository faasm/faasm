#include "State.h"
#include "RedisStateKeyValue.h"
#include "InMemoryStateKeyValue.h"

#include <util/config.h>
#include <util/locks.h>
#include <util/state.h>

#include <unistd.h>

using namespace util;

namespace state {
    State &getGlobalState() {
        static State s;
        return s;
    }

    // NOTE - to ease testing we must be careful to ensure the IP
    // used throughout the in-memory state is only captured here
    State::State() : thisIP(util::getSystemConfig().endpointHost) {

    }

    void State::forceClearAll(bool global) {
        std::string stateMode = util::getSystemConfig().stateMode;
        if (stateMode == "redis") {
            RedisStateKeyValue::clearAll(global);
        } else if (stateMode == "inmemory") {
            InMemoryStateKeyValue::clearAll(global);
        } else {
            throw std::runtime_error("Unrecognised state mode: " + stateMode);
        }

        util::SharedLock sharedLock(mapMutex);
        kvMap.clear();
    }

    size_t State::getStateSize(const std::string &user, const std::string &keyIn) {
        if (user.empty()) {
            throw std::runtime_error("Attempting to access state with empty user");
        }

        std::string lookupKey = util::keyForUser(user, keyIn);

        // See if we have the value locally
        {
            util::SharedLock sharedLock(mapMutex);
            if (kvMap.count(lookupKey) > 0) {
                return kvMap[lookupKey]->size();
            }
        }

        // Full lock
        FullLock fullLock(mapMutex);

        // Double check
        if (kvMap.count(lookupKey) > 0) {
            return kvMap[lookupKey]->size();
        }

        // Get from remote
        // TODO - cache this?
        std::string stateMode = util::getSystemConfig().stateMode;
        if (stateMode == "redis") {
            return RedisStateKeyValue::getStateSizeFromRemote(user, keyIn);
        } else if (stateMode == "inmemory") {
            return InMemoryStateKeyValue::getStateSizeFromRemote(user, keyIn, thisIP);
        } else {
            throw std::runtime_error("Unrecognised state mode: " + stateMode);
        }
    }

    std::shared_ptr<StateKeyValue> State::getKV(const std::string &user, const std::string &key, size_t size) {
        if (user.empty()) {
            throw std::runtime_error("Attempting to access state with empty user");
        }

        std::string lookupKey = util::keyForUser(user, key);

        // See if we have locally
        {
            SharedLock sharedLock(mapMutex);
            if (kvMap.count(lookupKey) > 0) {
                return kvMap[lookupKey];
            }
        }

        // Full lock
        FullLock fullLock(mapMutex);

        // Double check condition
        if (kvMap.count(lookupKey) > 0) {
            return kvMap[lookupKey];
        }

        // Sanity check on size
        if (size == 0) {
            throw StateKeyValueException("Attempting to get uninitialised value for key " + lookupKey);
        }

        // Create new KV
        std::string stateMode = util::getSystemConfig().stateMode;
        if (stateMode == "redis") {
            auto kv = new RedisStateKeyValue(user, key, size);
            kvMap.emplace(lookupKey, kv);
        } else if (stateMode == "inmemory") {
            // NOTE - passing IP here is crucial for testing
            auto kv = new InMemoryStateKeyValue(user, key, size, thisIP);
            kvMap.emplace(lookupKey, kv);
        } else {
            throw std::runtime_error("Unrecognised state mode: " + stateMode);
        }

        return kvMap[lookupKey];
    }

    size_t State::getKVCount() {
        return kvMap.size();
    }
}