#pragma once

#include "UserState.h"

#include <string>
#include <shared_mutex>

namespace state {
    class State {
    public:
        State();

        ~State();

        StateKeyValue *getKV(const std::string &user, const std::string &key, size_t size);

        UserState *getUserState(const std::string &user);

        void pushLoop();

        void pushAll();

        void forceClearAll();

    private:
        UserStateMap userStateMap;
        std::shared_mutex userStateMapMutex;

        long pushInterval;
    };

    State &getGlobalState();
}