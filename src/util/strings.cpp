#include "strings.h"

#include <string>
#include <boost/algorithm/string.hpp>

namespace util {
    std::vector<std::string> tokeniseString(const std::string &input, char delimiter) {
        std::vector<std::string> results;

        // Create a trimmed copy
        std::string copy(input);
        boost::trim(copy);

        boost::split(results, copy, [delimiter](char c) { return c == delimiter; });

        return results;
    }

    bool isAllWhitespace(const std::string &input) {
        return std::all_of(input.begin(), input.end(), isspace);
    }

    bool startsWith(const std::string &input, const std::string &subStr) {
        if (subStr.empty()) {
            return false;
        }

        return input.rfind(subStr, 0) == 0;
    }

    bool endsWith(std::string const &value, std::string const &ending) {
        if (ending.size() == 0) {
            return false;
        }
        else if (ending.size() > value.size()) {
            return false;
        }
        return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
    }

}
