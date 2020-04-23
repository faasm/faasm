#pragma once

#include "RedisStateKeyValue.h"

#include <util/clock.h>

namespace state {
    /**
    * Holds state for a given user
    */
    class UserState {
    public:
        explicit UserState(const std::string &userIn);

        size_t getSize(const std::string &key);

        // TODO - split up
        std::shared_ptr<RedisStateKeyValue> getValue(const std::string &key, size_t size);

        size_t getKeyCount();

    private:
        const std::string user;

        // TODO - genericise
        std::unordered_map<std::string, std::shared_ptr<RedisStateKeyValue>> kvMap;
        std::shared_mutex kvMapMutex;
    };

    typedef std::unordered_map<std::string, std::shared_ptr<UserState>> UserStateMap;
    typedef std::pair<std::string, std::shared_ptr<UserState>> UserStatePair;
}