#include "util.h"

#include <string>

namespace util {
    std::string getEnvVar(std::string const &key, std::string const &dflt) {
        char const *val = getenv(key.c_str());

        if(val == nullptr) {
            return dflt;
        }

        std::string retVal(val);

        if (retVal.length() == 0) {
            return dflt;
        }
        else {
            return retVal;
        }
    }
}