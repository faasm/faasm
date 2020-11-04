#pragma once

#include <string>
#define BASE_NETNS_NAME "faasmns"

namespace isolation {
enum NetworkIsolationMode
{
    ns_off,
    ns_on
};

class NetworkNamespace
{
  public:
    explicit NetworkNamespace(const std::string& name);

    void addCurrentThread();

    void removeCurrentThread();

    const std::string getName();

    const NetworkIsolationMode getMode();

  private:
    std::string name;
    NetworkIsolationMode mode;
};
}
