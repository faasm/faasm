#include "network.h"

#include <stdexcept>
#include <netdb.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <util/strings.h>


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

    /**
     * Returns the first IP found from any network interface with an
     * IPV4 address, unless an interface is supplied
     */
    std::string getPrimaryIPForThisHost(const std::string &interface) {
        // Get interfaces and addresses
        struct ifaddrs *allAddrs = nullptr;
        ::getifaddrs(&allAddrs);

        // Iterate through results
        struct ifaddrs *ifa = nullptr;
        std::string ipAddress;
        for (ifa = allAddrs; ifa != nullptr; ifa = ifa->ifa_next) {
            // IPV4 only
            if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET) {
                continue;
            }

            // Get the IP
            void *addr = &((struct sockaddr_in *) ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            ::inet_ntop(AF_INET, addr, addressBuffer, INET_ADDRSTRLEN);

            std::string ifaceName(ifa->ifa_name);

            // Skip loopback
            if (ifaceName == "lo") {
                continue;
            }

            if (interface.empty()) {
                // If interface not specified, attempt to work it out
                if (util::startsWith(ifaceName, "eth") ||
                    util::startsWith(ifaceName, "wl") ||
                    util::startsWith(ifaceName, "en")
                        ) {
                    ipAddress = addressBuffer;
                    break;
                }
            } else if (ifaceName == interface) {
                // If we have an interface specified, take that one
                ipAddress = std::string(addressBuffer);
                break;
            } else {
                continue;
            }
        }

        if (allAddrs != nullptr) {
            ::freeifaddrs(allAddrs);
        }

        if (ipAddress.empty()) {
            fprintf(stderr, "Unable to detect IP for this host");
        } else {
            printf("Detected IP %s\n", ipAddress.c_str());
        }

        return ipAddress;
    }
}
