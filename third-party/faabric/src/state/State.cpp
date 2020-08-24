#include "State.h"
#include "RedisStateKeyValue.h"
#include "InMemoryStateKeyValue.h"

#include <faabric/util/config.h>
#include <faabric/util/locks.h>
#include <faabric/util/state.h>
#include <faabric/util/logging.h>

#include <unistd.h>

using namespace faabric::util;

namespace faabric::state {
    State &getGlobalState() {
        static State s(faabric::utilgetSystemConfig().endpointHost);
        return s;
    }

    State::State(std::string thisIPIn) : thisIP(thisIPIn) {

    }

    void State::forceClearAll(bool global) {
        std::string stateMode = faabric::utilgetSystemConfig().stateMode;
        if (stateMode == "redis") {
            RedisStateKeyValue::clearAll(global);
        } else if (stateMode == "inmemory") {
            InMemoryStateKeyValue::clearAll(global);
        } else {
            throw std::runtime_error("Unrecognised state mode: " + stateMode);
        }

        faabric::utilSharedLock sharedLock(mapMutex);
        kvMap.clear();
    }

    size_t State::getStateSize(const std::string &user, const std::string &keyIn) {
        if (user.empty()) {
            throw std::runtime_error("Attempting to access state with empty user");
        }

        std::string lookupKey = faabric::utilkeyForUser(user, keyIn);

        // See if we have the value locally
        {
            faabric::utilSharedLock sharedLock(mapMutex);
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
        std::string stateMode = faabric::utilgetSystemConfig().stateMode;
        if (stateMode == "redis") {
            return RedisStateKeyValue::getStateSizeFromRemote(user, keyIn);
        } else if (stateMode == "inmemory") {
            return InMemoryStateKeyValue::getStateSizeFromRemote(user, keyIn, thisIP);
        } else {
            throw std::runtime_error("Unrecognised state mode: " + stateMode);
        }
    }

    void State::deleteKV(const std::string &userIn, const std::string &keyIn) {
        std::string stateMode = faabric::utilgetSystemConfig().stateMode;
        if (stateMode == "redis") {
            RedisStateKeyValue::deleteFromRemote(userIn, keyIn);
        } else if (stateMode == "inmemory") {
            InMemoryStateKeyValue::deleteFromRemote(userIn, keyIn, thisIP);
        } else {
            throw std::runtime_error("Unrecognised state mode: " + stateMode);
        }

        deleteKVLocally(userIn, keyIn);
    }

    void State::deleteKVLocally(const std::string &userIn, const std::string &keyIn) {
        FullLock fullLock(mapMutex);
        std::string lookupKey = faabric::utilkeyForUser(userIn, keyIn);
        kvMap.erase(lookupKey);
    }

    std::shared_ptr<StateKeyValue> State::getKV(const std::string &user, const std::string &key) {
        return doGetKV(user, key, true, 0);
    }


    std::shared_ptr<StateKeyValue> State::getKV(const std::string &user, const std::string &key, size_t size) {
        return doGetKV(user, key, false, size);
    }

    std::shared_ptr<StateKeyValue>
    State::doGetKV(const std::string &user, const std::string &key, bool sizeless, size_t size) {
        if (user.empty() || key.empty()) {
            throw std::runtime_error(
                    fmt::format("Attempting to access state with empty user or key ({}/{})", user, key)
            );
        }

        std::string lookupKey = faabric::utilkeyForUser(user, key);

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

        // Sanity check on size if not sizeless
        if (!sizeless && size == 0) {
            throw StateKeyValueException("Must specify size for creating key-value " + lookupKey);
        }

        // Create new KV
        std::string stateMode = faabric::utilgetSystemConfig().stateMode;
        if (stateMode == "redis") {
            if (sizeless) {
                auto kv = new RedisStateKeyValue(user, key);
                kvMap.emplace(lookupKey, kv);
            } else {
                auto kv = new RedisStateKeyValue(user, key, size);
                kvMap.emplace(lookupKey, kv);
            }
        } else if (stateMode == "inmemory") {
            // NOTE - passing IP here is crucial for testing
            if (sizeless) {
                auto kv = new InMemoryStateKeyValue(user, key, thisIP);
                kvMap.emplace(lookupKey, kv);
            } else {
                auto kv = new InMemoryStateKeyValue(user, key, size, thisIP);
                kvMap.emplace(lookupKey, kv);
            }
        } else {
            throw std::runtime_error("Unrecognised state mode: " + stateMode);
        }

        return kvMap[lookupKey];
    }

    size_t State::getKVCount() {
        return kvMap.size();
    }

    std::string State::getThisIP() {
        return thisIP;
    }
}
