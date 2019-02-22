#include "CGroup.h"

#include <util/environment.h>
#include <util/logging.h>
#include <util/config.h>

#include <mutex>

#include <syscall.h>
#include <sys/types.h>
#include <boost/filesystem.hpp>

using namespace boost::filesystem;

namespace worker {
    static const std::string BASE_DIR = "/sys/fs/cgroup/";
    static const std::string CG_CPU = "cpu";

    static const std::vector<std::string> controllers = {CG_CPU};

    static std::mutex groupMutex;

    CGroup::CGroup(const std::string &name) : name(name) {
        util::SystemConfig &conf = util::getSystemConfig();

        if (conf.cgroupMode == "on") {
            mode = CgroupMode::cg_on;
        } else {
            mode = CgroupMode::cg_off;
        }
    }

    const std::string CGroup::getName() {
        return this->name;
    }

    const CgroupMode CGroup::getMode() {
        return this->mode;
    }

    pid_t getCurrentTid() {
        auto tid = (pid_t) syscall(SYS_gettid);
        return tid;
    }

    void addCurrentThreadToTasks(const path &tasksPath) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        pid_t threadId = getCurrentTid();

        std::ofstream outfile;
        outfile.open(tasksPath.string(), std::ios_base::app);
        outfile << threadId << std::endl;
        outfile.flush();

        logger->debug("Added thread id {} to {}", threadId, tasksPath.string());
    }

    void CGroup::addCurrentThread() {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if (mode == CgroupMode::cg_off) {
            logger->debug("Not adding thread. cgroup support off");
            return;
        }

        // Get lock and add to controllers
        std::scoped_lock<std::mutex> guard(groupMutex);

        for (const std::string &controller : controllers) {
            path tasksPath(BASE_DIR);
            tasksPath.append(controller);
            tasksPath.append(this->name);
            tasksPath.append("tasks");

            addCurrentThreadToTasks(tasksPath);
        }
    }
}
