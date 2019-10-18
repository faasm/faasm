#pragma once

#include "UserState.h"

#include <string>
#include <shared_mutex>

namespace state {
    class State {
    public:
        std::shared_ptr<StateKeyValue> getKV(const std::string &user, const std::string &key, size_t size);

        std::shared_ptr<UserState> getUserState(const std::string &user);

        void forceClearAll();

    private:
        UserStateMap userStateMap;
        std::shared_mutex userStateMapMutex;
    };

    State &getGlobalState();
}