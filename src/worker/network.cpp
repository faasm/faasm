#include "worker.h"

#include <errno.h>
#include <fcntl.h>

namespace worker {

    static std::string BASE_NS = "faasm";

    NetworkNamespace::NetworkNamespace(int index) {
        name = BASE_NS + std::to_string(index);

        // Get which mode we're operating in
        std::string modeEnv = util::getEnvVar("NETNS_MODE", "on");

        if (modeEnv == "on") {
            mode = NetworkIsolationMode::ns_on;
        } else {
            mode = NetworkIsolationMode::ns_off;
        }
    };

    void NetworkNamespace::addCurrentThread() {
        if(mode == NetworkIsolationMode::ns_off) {
            std::cout << "Not using network ns, support off" << std::endl;
            return;
        }

        std::cout << "Adding thread to network ns " << name << std::endl;

        std::string ns_path = "/var/run/netns/" + name;
        int fd = open(ns_path.c_str(), O_RDONLY);

        int result = setns(fd, CLONE_NEWNET);

        if(result == 0) {
            std::cout << "Setns succeeded";
            sleep(300);
            return;
        }

        std::cout << "Setns failed: " << errno << std::endl;
    };
}
