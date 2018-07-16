#include "util.h"
#include <string>

namespace util {
    std::string getEnvVar(std::string const &key, std::string const &dflt) {
        char const *val = getenv(key.c_str());
        return val == nullptr ? dflt : std::string(val);
    }
}