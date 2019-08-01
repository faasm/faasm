#include "State.h"
#include "UserState.h"
#include "StateKeyValue.h"

#include <util/config.h>
#include <util/locks.h>

#include <unistd.h>

using namespace util;

namespace state {
    /**
     * Global state (can hold many users' state)
     */

    State &getGlobalState() {
        static State s;
        return s;
    }

    void State::pushAll() {
        // Run the sync for all users' state
        for (const auto &iter: userStateMap) {
            iter.second->pushAll();
        }
    }

    void State::forceClearAll() {
        userStateMap.clear();
    }

    std::shared_ptr<StateKeyValue> State::getKV(const std::string &user, const std::string &key, size_t size) {
        if(user.empty()) {
            throw std::runtime_error("Attempting to access state with empty user");
        }

        std::shared_ptr<UserState> us = this->getUserState(user);
        return us->getValue(key, size);
    }

    std::shared_ptr<UserState> State::getUserState(const std::string &user) {
        if (userStateMap.count(user) == 0) {
            // Lock on editing user state registry
            FullLock fullLock(userStateMapMutex);

            // Double check it still doesn't exist
            if (userStateMap.count(user) == 0) {
                auto s = new UserState(user);

                userStateMap.emplace(UserStatePair(user, s));
            }
        }

        return userStateMap[user];
    }
}