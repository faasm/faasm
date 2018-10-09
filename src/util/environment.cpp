#include "util.h"

#include <string>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <limits.h>

namespace util {
    std::string getEnvVar(std::string const &key, std::string const &deflt) {
        char const *val = getenv(key.c_str());

        if(val == nullptr) {
            return deflt;
        }

        std::string retVal(val);

        if (retVal.length() == 0) {
            return deflt;
        }
        else {
            return retVal;
        }
    }

    void setEnvVar(const std::string &varName, const std::string &value) {
        setenv(varName.c_str(), value.c_str(), 1);
    }

    void unsetEnvVar(const std::string &varName) {
        unsetenv(varName.c_str());
    }

    std::string getCurrentHostname() {
        char hostname[HOST_NAME_MAX];
        gethostname(hostname, HOST_NAME_MAX);

        std::string result(hostname);

        return result;
    }
}