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

    std::shared_ptr<StateKeyValue> UserState::getValue(const std::string &key, size_t size) {
        if (kvMap.count(key) == 0) {
            if (size == 0) {
                throw StateKeyValueException("Uninitialized value for key " + key);
            }

            // Lock on editing local state registry
            FullLock fullLock(kvMapMutex);

            // Always mask keys with the user
            std::string actualKey = util::keyForUser(user, key);

            // Double check it still doesn't exist
            if (kvMap.count(key) == 0) {
                auto kv = new StateKeyValue(actualKey, size);

                kvMap.emplace(KVPair(key, kv));
            }
        }

        return kvMap[key];
    }
}