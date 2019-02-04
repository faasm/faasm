#pragma once

#include <string>

namespace util {
    std::string b64encode(const std::string &bytes);

    std::string b64decode(const std::string &bytes);
}