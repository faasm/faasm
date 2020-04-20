#pragma once

#include "UserState.h"

#include <string>
#include <shared_mutex>

namespace state {
    class State {
    public:
        size_t getStateSize(const std::string &user, const std::string &keyIn);

        std::shared_ptr<StateKeyValue> getKV(const std::string &user, const std::string &key, size_t size);

        std::shared_ptr<UserState> getUserState(const std::string &user);

        void forceClearAll();

        size_t getKVCount();
    private:
        UserStateMap userStateMap;
        std::shared_mutex userStateMapMutex;
    };

    State &getGlobalState();
}