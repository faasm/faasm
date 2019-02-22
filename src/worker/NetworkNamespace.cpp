#include "NetworkNamespace.h"

#include <boost/filesystem.hpp>

#include <util/environment.h>
#include <util/logging.h>
#include <util/config.h>

#include <errno.h>
#include <fcntl.h>


namespace worker {
    NetworkNamespace::NetworkNamespace(const std::string &name) : name(name) {
        // Get which mode we're operating in
        util::SystemConfig &conf = util::getSystemConfig();

        if (conf.netNsMode == "on") {
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
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        logger->debug("Setting network ns to {}", nsPath.string());

        int fd;
        fd = open(nsPath.c_str(), O_RDONLY, 0);
        if(fd == -1) {
            std::string errorMsg = "Failed to open fd at " + nsPath.string();
            throw std::runtime_error(errorMsg);
        }

        int result = setns(fd, 0);
        close(fd);

        if (result != 0) {
            std::string errorMsg = "setns failed " + std::to_string(errno);
            throw std::runtime_error(errorMsg);
        }
    }

    void NetworkNamespace::addCurrentThread() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if (mode == NetworkIsolationMode::ns_off) {
            logger->debug("Not using network ns, support off");
            return;
        }

        logger->debug("Adding thread to network ns: {}", name);

        // Open path to the namespace
        boost::filesystem::path nsPath("/var/run/netns");
        nsPath.append(name);

        joinNamespace(nsPath);
    };

    void NetworkNamespace::removeCurrentThread() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if (mode == NetworkIsolationMode::ns_off) {
            logger->debug("Not using network ns, support off");
            return;
        }

        // Return process to its parent namespace
        pid_t parentPid = getppid();

        boost::filesystem::path nsPath("/proc");
        nsPath.append(std::to_string(parentPid));
        nsPath.append("ns/net");

        joinNamespace(nsPath);
    }
}
