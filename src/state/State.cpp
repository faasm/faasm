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

//    State::~State() {
//        this->forceClearAll();
//    }

    void State::pushAll() {
        // Run the sync for all users' state
        for (const auto &iter: userStateMap) {
            iter.second->pushAll();
        }
    }

    void State::forceClearAll() {
//        if(!userStateMap.empty()) {
//            // Delete contents of user state map
//            for (const auto &iter: userStateMap) {
//                delete iter.second;
//            }
//        }
//        userStateMap.clear();
    }

    StateKeyValue *State::getKV(const std::string &user, const std::string &key, size_t size) {
        UserState *us = this->getUserState(user);
        return us->getValue(key, size);
    }

    UserState *State::getUserState(const std::string &user) {
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