#include "NetworkNamespace.h"

#include <boost/filesystem.hpp>

#include <faabric/util/config.h>
#include <faabric/util/environment.h>
#include <faabric/util/locks.h>
#include <faabric/util/logging.h>
#include <faabric/util/timing.h>

#include <errno.h>
#include <fcntl.h>

namespace isolation {

std::vector<std::shared_ptr<NetworkNamespace>> namespaces;
bool namespacesInitialised = false;
std::mutex namespacesLock;

void returnNetworkNamespace(std::shared_ptr<NetworkNamespace> ns)
{
    faabric::util::UniqueLock lock(namespacesLock);
    namespaces.emplace_back(ns);
}

std::shared_ptr<NetworkNamespace> claimNetworkNamespace()
{
    if (namespaces.empty() && !namespacesInitialised) {
        faabric::util::UniqueLock lock(namespacesLock);

        if (namespaces.empty() && !namespacesInitialised) {
            namespacesInitialised = true;

            // Note that the availability of namespaces depends on the Faasm
            // configuration for the relevant host
            // TODO - connect these two up properly
            int nNamespaces = faabric::util::getUsableCores() * 10;

            for (int i = 0; i < nNamespaces; i++) {
                std::string netnsName = BASE_NETNS_NAME + std::to_string(i);
                namespaces.emplace_back(
                  std::make_shared<NetworkNamespace>(netnsName));
            }
        }
    } else if (namespaces.empty()) {
        throw std::runtime_error("Namespaces have run out");
    }

    faabric::util::UniqueLock lock(namespacesLock);

    std::shared_ptr<NetworkNamespace> res = namespaces.back();
    namespaces.pop_back();
    return res;
}

NetworkNamespace::NetworkNamespace(const std::string& name)
  : name(name)
{
    // Get which mode we're operating in
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();

    if (conf.netNsMode == "on") {
        mode = NetworkIsolationMode::ns_on;
    } else {
        mode = NetworkIsolationMode::ns_off;
    }
};

const std::string NetworkNamespace::getName()
{
    return this->name;
}

const NetworkIsolationMode NetworkNamespace::getMode()
{
    return this->mode;
}

void joinNamespace(const boost::filesystem::path& nsPath)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();

    logger->debug("Setting network ns to {}", nsPath.string());

    int fd;
    fd = open(nsPath.c_str(), O_RDONLY, 0);
    if (fd == -1) {
        std::string errorMsg = "Failed to open fd at " + nsPath.string();
        throw std::runtime_error(errorMsg);
    }

    int result = setns(fd, 0);
    close(fd);

    if (result != 0) {
        logger->error("Failed to join namespace at {} - {}",
                      nsPath.string(),
                      std::strerror(errno));
        std::string errorMsg = "setns failed " + std::to_string(errno);
        throw std::runtime_error(errorMsg);
    }
}

void NetworkNamespace::addCurrentThread()
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();

    if (mode == NetworkIsolationMode::ns_off) {
        logger->debug("Not using network ns, support off");
        return;
    }

    PROF_START(netNsAdd)
    logger->debug("Adding thread to network ns: {}", name);

    // Open path to the namespace
    boost::filesystem::path nsPath("/var/run/netns");
    nsPath.append(name);

    joinNamespace(nsPath);
    PROF_END(netNsAdd)
};

void NetworkNamespace::removeCurrentThread()
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();

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
