#pragma once

#include <string>

namespace util {
    std::string getEnvVar(std::string const &key, std::string const &dflt);
}
