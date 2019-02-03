#pragma once

#include <string>

namespace util {
    std::string getEnvVar(const std::string &key, const std::string &deflt);

    void setEnvVar(const std::string &varName, const std::string &value);

    void unsetEnvVar(const std::string &varName);
}