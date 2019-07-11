#include "state.h"

namespace util {
    std::string keyForUser(const std::string &user, const std::string &key) {
        if(user.empty()) {
            return key;
        }

        std::string fullKey = user + "_" + key;

        return fullKey;
    }
}

