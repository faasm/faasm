#pragma once

#include "StateKeyValue.h"

#include <string>
#include <shared_mutex>

namespace state {
    class State {
    public:
        State();

        size_t getStateSize(const std::string &user, const std::string &keyIn);

        std::shared_ptr<StateKeyValue> getKV(const std::string &user, const std::string &key, size_t size);

        void forceClearAll(bool global);

        size_t getKVCount();
    private:
        const std::string thisIP;

        std::unordered_map<std::string, std::shared_ptr<StateKeyValue>> kvMap;
        std::shared_mutex mapMutex;
    };

    State &getGlobalState();
}