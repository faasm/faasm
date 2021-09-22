#pragma once

#include <memory>
#include <string>

#define BASE_NETNS_NAME "faasmns"

namespace isolation {
class NetworkNamespace
{
  public:
    explicit NetworkNamespace(const std::string& name);

    void addCurrentThread();

    void removeCurrentThread();

    const std::string getName();

  private:
    std::string name;
};

std::shared_ptr<NetworkNamespace> claimNetworkNamespace();

void returnNetworkNamespace(std::shared_ptr<NetworkNamespace> ns);

}
