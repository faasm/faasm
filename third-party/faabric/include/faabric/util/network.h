#pragma once

#include <string>

namespace faabric::util {
    std::string getIPFromHostname(const std::string &hostname);

    std::string getPrimaryIPForThisHost(const std::string &interface);
}