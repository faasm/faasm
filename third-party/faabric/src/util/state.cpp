#include "state.h"

#include <util/logging.h>

namespace util {
    std::string keyForUser(const std::string &user, const std::string &key) {
        if (user.empty() || key.empty()) {
            throw std::runtime_error(fmt::format("Cannot have empty user or key ({}/{})", user, key));
        }

        std::string fullKey = user + "_" + key;

        return fullKey;
    }
}

