#include "util/util.h"

#include <netdb.h>
#include <arpa/inet.h>

namespace util {
    std::string getIPFromHostname(const std::string &hostname) {
        hostent *record = gethostbyname(hostname.c_str());

        if (record == nullptr) {
            std::string errorMsg = "Could not resolve host " + hostname;
            throw std::runtime_error(errorMsg);
        }

        auto address = (in_addr *) record->h_addr;
        std::string ipAddress = inet_ntoa(*address);

        return ipAddress;
    }
}
