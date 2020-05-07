#pragma once

#include "StateKeyValue.h"

#include <string>
#include <shared_mutex>

namespace state {
    class State {
    public:
        explicit State(std::string thisIPIn);

        size_t getStateSize(const std::string &user, const std::string &keyIn);

        std::shared_ptr<StateKeyValue> getKV(const std::string &user, const std::string &key, size_t size);

        void forceClearAll(bool global);

        void deleteKV(const std::string &userIn, const std::string &keyIn);

        size_t getKVCount();
    private:
        const std::string thisIP;

        std::unordered_map<std::string, std::shared_ptr<StateKeyValue>> kvMap;
        std::shared_mutex mapMutex;
    };

    State &getGlobalState();
}