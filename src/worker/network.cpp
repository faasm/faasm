#include "worker.h"

#include <boost/filesystem.hpp>

#include <errno.h>
#include <fcntl.h>


namespace worker {

    NetworkNamespace::NetworkNamespace(const std::string &name) : name(name) {
        // Get which mode we're operating in
        std::string modeEnv = util::getEnvVar("NETNS_MODE", "on");

        if (modeEnv == "on") {
            mode = NetworkIsolationMode::ns_on;
        } else {
            mode = NetworkIsolationMode::ns_off;
        }
    };

    const std::string NetworkNamespace::getName() {
        return this->name;
    }

    const NetworkIsolationMode NetworkNamespace::getMode() {
        return this->mode;
    }

    /** Joins the namespace at the given path */
    void joinNamespace(const boost::filesystem::path &nsPath) {
        std::cout << "Opening fd at " << nsPath << std::endl;
        int fd = open(nsPath.c_str(), O_RDONLY, 0);

        std::cout << "Setting ns" << std::endl;
        int result = setns(fd, CLONE_NEWNET);

        std::cout << "Closing fd " << nsPath << std::endl;
        close(fd);

        if(result == 0) {
            std::cout << "Setns succeeded" << std::endl;
            return;
        }

        std::cout << "Setns failed: " << errno << std::endl;
    }

    void NetworkNamespace::addCurrentThread() {
        if(mode == NetworkIsolationMode::ns_off) {
            std::cout << "Not using network ns, support off" << std::endl;
            return;
        }

        std::cout << "Adding thread to network ns " << name << std::endl;

        // Open path to the namespace
        boost::filesystem::path nsPath("/var/run/netns");
        nsPath.append(name);

        joinNamespace(nsPath);
    };

    void NetworkNamespace::removeCurrentThread() {
        if(mode == NetworkIsolationMode::ns_off) {
            std::cout << "Not using network ns, support off" << std::endl;
            return;
        }

        // Return process to its parent namespace
        pid_t parentPid = getppid();

        boost::filesystem::path nsPath("/proc");
        nsPath.append(std::to_string(parentPid));
        nsPath.append("net/ns");

        joinNamespace(nsPath);
    }


}
