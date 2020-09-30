#pragma once

#include <string>
#define BASE_CGROUP_NAME "faasm"

namespace isolation {
enum CgroupMode
{
    cg_off,
    cg_on
};

class CGroup
{
  public:
    explicit CGroup(const std::string& name);

    void addCurrentThread();

    const std::string getName();

    const CgroupMode getMode();

  private:
    std::string name;
    CgroupMode mode;
};
}