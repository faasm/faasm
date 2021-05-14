#include "CGroup.h"

#include <conf/FaasmConfig.h>

#include <faabric/util/config.h>
#include <faabric/util/logging.h>
#include <faabric/util/timing.h>

#include <mutex>
#include <syscall.h>

#include <boost/filesystem.hpp>

using namespace boost::filesystem;

namespace isolation {
static const std::string BASE_DIR = "/sys/fs/cgroup/";
static const std::string CG_CPU = "cpu";

static const std::vector<std::string> controllers = { CG_CPU };

static std::mutex groupMutex;

CGroup::CGroup(const std::string& name)
  : name(name)
{
    conf::FaasmConfig& conf = conf::getFaasmConfig();

    if (conf.cgroupMode == "on") {
        mode = CgroupMode::cg_on;
    } else {
        mode = CgroupMode::cg_off;
    }
}

const std::string CGroup::getName()
{
    return this->name;
}

const CgroupMode CGroup::getMode()
{
    return this->mode;
}

pid_t getCurrentTid()
{
    auto tid = (pid_t)syscall(SYS_gettid);
    return tid;
}

void addCurrentThreadToTasks(const path& tasksPath)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();

    pid_t threadId = getCurrentTid();

    std::ofstream outfile;
    outfile.open(tasksPath.string(), std::ios_base::app);
    outfile << threadId << std::endl;
    outfile.flush();

    logger->debug("Added thread id {} to {}", threadId, tasksPath.string());
}

void CGroup::addCurrentThread()
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();

    if (mode == CgroupMode::cg_off) {
        logger->debug("Not adding thread. cgroup support off");
        return;
    }

    PROF_START(cGroupAdd)
    // Get lock and add to controllers
    std::scoped_lock<std::mutex> guard(groupMutex);

    for (const std::string& controller : controllers) {
        path tasksPath(BASE_DIR);
        tasksPath.append(controller);
        tasksPath.append(this->name);
        tasksPath.append("tasks");

        addCurrentThreadToTasks(tasksPath);
    }
    PROF_END(cGroupAdd)
}
}
