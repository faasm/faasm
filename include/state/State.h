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

        void pushAll();

        void forceClearAll();

        void shutdown();
    private:
        std::atomic<bool> _shutdown;
        UserStateMap userStateMap;
        std::shared_mutex userStateMapMutex;

        long pushInterval;
    };

    State &getGlobalState();
}