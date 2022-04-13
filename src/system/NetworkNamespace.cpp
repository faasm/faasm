#include "NetworkNamespace.h"

#include <boost/filesystem.hpp>

#include <faabric/util/config.h>
#include <faabric/util/environment.h>
#include <faabric/util/locks.h>
#include <faabric/util/logging.h>
#include <faabric/util/timing.h>

#include <conf/FaasmConfig.h>

#include <errno.h>
#include <fcntl.h>

namespace isolation {

std::vector<std::shared_ptr<NetworkNamespace>> namespaces;
bool namespacesInitialised = false;
std::mutex namespacesLock;

void returnNetworkNamespace(std::shared_ptr<NetworkNamespace> ns)
{
    const auto& conf = conf::getFaasmConfig();
    if (conf.netNsMode == "off") {
        return;
    }

    faabric::util::UniqueLock lock(namespacesLock);
    namespaces.emplace_back(ns);
}

std::shared_ptr<NetworkNamespace> claimNetworkNamespace()
{
    faabric::util::UniqueLock lock(namespacesLock);
    const auto& conf = conf::getFaasmConfig();

    if (namespaces.empty() && !namespacesInitialised) {
        if (conf.maxNetNs == 0) {
            SPDLOG_ERROR("Can't provide a MAX_NET_NAMESPACES value of 0");
            throw std::runtime_error("Invalid env. var MAX_NET_NAMESPACES");
        }

        for (int i = 0; i < conf.maxNetNs; i++) {
            std::string netnsName = BASE_NETNS_NAME + std::to_string(i);
            namespaces.emplace_back(
              std::make_shared<NetworkNamespace>(netnsName));
        }

        namespacesInitialised = true;
    }

    if (namespaces.empty()) {
        throw std::runtime_error("Namespaces have run out");
    }

    std::shared_ptr<NetworkNamespace> res = namespaces.back();

    // If network namespaces are turned off, we return a valid pointer but
    // don't remove the resource, as no isolation is actually taking place
    if (conf.netNsMode == "off") {
        return res;
    }

    namespaces.pop_back();
    return res;
}

NetworkNamespace::NetworkNamespace(const std::string& name)
  : name(name){};

const std::string NetworkNamespace::getName()
{
    faabric::util::SharedLock lock(mx);
    return this->name;
}

void joinNamespace(const boost::filesystem::path& nsPath)
{
    const auto& conf = conf::getFaasmConfig();
    if (conf.netNsMode == "off") {
        return;
    }

    SPDLOG_DEBUG("Setting network ns to {}", nsPath.string());

    int fd;
    fd = open(nsPath.c_str(), O_RDONLY, 0);
    if (fd == -1) {
        std::string errorMsg = "Failed to open fd at " + nsPath.string();
        throw std::runtime_error(errorMsg);
    }

    int result = setns(fd, 0);
    close(fd);

    if (result != 0) {
        SPDLOG_ERROR("Failed to join namespace at {} - {}",
                     nsPath.string(),
                     std::strerror(errno));
        std::string errorMsg = "setns failed " + std::to_string(errno);
        throw std::runtime_error(errorMsg);
    }
}

void NetworkNamespace::addCurrentThread()
{
    faabric::util::FullLock lock(mx);
    const auto& conf = conf::getFaasmConfig();
    if (conf.netNsMode == "off") {
        SPDLOG_DEBUG("Not using network ns, support off");
        return;
    }

    PROF_START(netNsAdd)
    SPDLOG_DEBUG("Adding thread to network ns: {}", name);

    // Open path to the namespace
    boost::filesystem::path nsPath("/var/run/netns");
    nsPath.append(name);

    joinNamespace(nsPath);
    PROF_END(netNsAdd)
};

void NetworkNamespace::removeCurrentThread()
{
    faabric::util::FullLock lock(mx);
    const auto& conf = conf::getFaasmConfig();
    if (conf.netNsMode == "off") {
        SPDLOG_DEBUG("Not using network ns, support off");
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
