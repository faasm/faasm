#include "util.h"
#include <string>
#include <boost/algorithm/string.hpp>

namespace util {
    std::vector<std::string> tokeniseString(const std::string &input, char delimiter) {
        std::vector<std::string> results;
        boost::split(results, input, [delimiter](char c) { return c == delimiter; });

        return results;
    }

    bool isAllWhitespace(const std::string &input) {
        return std::all_of(input.begin(), input.end(), isspace);
    }
}
