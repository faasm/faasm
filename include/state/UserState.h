#pragma once

#include "StateKeyValue.h"

#include <util/clock.h>

namespace state {
    /**
    * Holds state for a given user
    */
    class UserState {
    public:
        explicit UserState(const std::string &userIn);

        size_t getSize(const std::string &key);

        std::shared_ptr<StateKeyValue> getValue(const std::string &key, size_t size);

        size_t getKeyCount();

    private:
        const std::string user;

        KVMap kvMap;
        std::shared_mutex kvMapMutex;
    };

    typedef std::unordered_map<std::string, std::shared_ptr<UserState>> UserStateMap;
    typedef std::pair<std::string, std::shared_ptr<UserState>> UserStatePair;
}