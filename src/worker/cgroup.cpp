#include "worker/worker.h"

#include <syscall.h>
#include <sys/types.h>
#include <boost/filesystem.hpp>

using namespace boost::filesystem;

namespace worker {
    static const std::string BASE_DIR = "/sys/fs/cgroup/";
    static const std::string CG_CPU = "cpu";

    static const std::vector<std::string> controllers = {CG_CPU};

    static std::mutex groupMutex;
    static std::mutex rootMutex;

    CGroup::CGroup(const std::string &name) : name(name) {
        // Get which cgroup mode we're operating in
        std::string modeEnv = util::getEnvVar("CGROUP_MODE", "on");

        if (modeEnv == "on") {
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
        pid_t threadId = getCurrentTid();

        std::ofstream outfile;
        outfile.open(tasksPath.string(), std::ios_base::app);
        outfile << threadId << std::endl;
        outfile.flush();

        std::cout << "Added thread id " << threadId << " to " << tasksPath << std::endl;
    }

    void CGroup::addCurrentThread() {

        if (mode == CgroupMode::cg_off) {
            std::cout << "Not adding thread. cgroup support off" << std::endl;
            return;
        }

        // Get lock and add to controllers
        std::lock_guard<std::mutex> guard(groupMutex);

        for (const std::string &controller : controllers) {
            path tasksPath(BASE_DIR);
            tasksPath.append(controller);
            tasksPath.append(this->name);
            tasksPath.append("tasks");

            addCurrentThreadToTasks(tasksPath);
        }
    }

    /** Note, to remove a thread from a given cgroup, you must add it to the tasks file in the root */
    void CGroup::removeCurrentThread() {
        if (mode == CgroupMode::cg_off) {
            std::cout << "Not removing thread. cgroup support off" << std::endl;
            return;
        }

        std::lock_guard<std::mutex> guard(rootMutex);

        for (const std::string &controller: controllers) {
            path tasksPath(BASE_DIR);
            tasksPath.append(controller);
            tasksPath.append("tasks");

            addCurrentThreadToTasks(tasksPath);
        }
    }

}
