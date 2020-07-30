#pragma once

#include <unordered_map>
#include <string>
#include <shared_mutex>


namespace state {
    class InMemoryStateRegistry {
    public:
        InMemoryStateRegistry();

        std::string getMasterIP(
                const std::string &user, const std::string &key,
                const std::string &thisIP,
                bool claim
        );

        std::string getMasterIPForOtherMaster(
                const std::string &userIn,
                const std::string &keyIn,
                const std::string &thisIP
        );

        void clear();

    private:
        static std::unordered_map<std::string, std::string> masterMap;
        static std::shared_mutex masterMapMutex;
    };

    InMemoryStateRegistry &getInMemoryStateRegistry();
}