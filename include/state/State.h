#pragma once

#include "StateKeyValue.h"

#include <string>
#include <shared_mutex>

#define STATE_HOST "0.0.0.0"
#define STATE_PORT 8003


namespace state {
    class State {
    public:
        explicit State(std::string thisIPIn);

        size_t getStateSize(const std::string &user, const std::string &keyIn);

        std::shared_ptr<StateKeyValue> getKV(const std::string &user, const std::string &key, size_t size);

        std::shared_ptr<StateKeyValue> getKV(const std::string &user, const std::string &key);

        void forceClearAll(bool global);

        void deleteKV(const std::string &userIn, const std::string &keyIn);

        void deleteKVLocally(const std::string &userIn, const std::string &keyIn);

        size_t getKVCount();

        std::string getThisIP();
    private:
        const std::string thisIP;

        std::unordered_map<std::string, std::shared_ptr<StateKeyValue>> kvMap;
        std::shared_mutex mapMutex;

        std::shared_ptr<StateKeyValue> doGetKV(const std::string &user, const std::string &key, bool sizeless, size_t size);
    };

    State &getGlobalState();
}