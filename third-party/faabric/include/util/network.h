#pragma once

#include <string>

namespace util {
    std::string getIPFromHostname(const std::string &hostname);

    std::string getPrimaryIPForThisHost(const std::string &interface);
}