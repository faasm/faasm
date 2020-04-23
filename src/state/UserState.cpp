#include "UserState.h"

#include <util/locks.h>
#include <util/state.h>

using namespace util;

namespace state {
    /**
     * User state (can have multiple key/ values)
     */

    UserState::UserState(const std::string &userIn) : user(userIn) {

    }

    size_t UserState::getKeyCount() {
        return kvMap.size();
    }

    size_t UserState::getSize(const std::string &key) {
        {
            util::SharedLock sharedLock(kvMapMutex);
            if (kvMap.count(key) > 0) {
                return kvMap[key]->size();
            }
        }

        // Full lock for doing anything
        FullLock fullLock(kvMapMutex);

        std::string actualKey = util::keyForUser(user, key);
        // TODO break hard Redis dep
        redis::Redis &redis = redis::Redis::getState();
        return redis.strlen(actualKey);
    }

    std::shared_ptr<RedisStateKeyValue> UserState::getValue(const std::string &key, size_t size) {
        {
            util::SharedLock sharedLock(kvMapMutex);
            if (kvMap.count(key) > 0) {
                return kvMap[key];
            }
        }

        // Full lock for doing anything
        FullLock fullLock(kvMapMutex);

        // Double check condition
        if (kvMap.count(key) > 0) {
            return kvMap[key];
        }

        if (size == 0) {
            throw StateKeyValueException("Uninitialized value for key " + key);
        }

        // Always mask keys with the user
        std::string actualKey = util::keyForUser(user, key);

        // TODO - work out how to do this with different types
        auto kv = new RedisStateKeyValue(actualKey, size);
        kvMap.emplace(key, kv);

        return kvMap[key];
    }
}