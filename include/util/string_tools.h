#pragma once

#include <string>
#include <vector>

namespace util {
    std::vector<std::string> splitString(const std::string &input, char delimiter);

    bool isAllWhitespace(const std::string &input);

    bool startsWith(const std::string &input, const std::string &subStr);

    bool endsWith(const std::string &input, const std::string &subStr);

    bool contains(const std::string &input, const std::string &subStr);

    std::string removeSubstr(const std::string &input, const std::string &toErase);

    bool stringIsInt(const std::string &input);
}