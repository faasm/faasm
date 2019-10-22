#pragma once

#include <unordered_set>

namespace util {
    std::string randomString(int len);

    std::string randomStringFromSet(const std::unordered_set<std::string> &s);
}