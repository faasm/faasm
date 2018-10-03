#include "worker/worker.h"

#include <syscall.h>
#include <sys/types.h>

namespace worker {
    static const std::string BASE_DIR = "/sys/fs/cgroup/";
    static const std::string CG_CPU = "cpu";
    static const std::string CG_NET_CLS = "net_cls";

    static std::mutex groupMutex;

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

    void CGroup::addCurrentThread() {
        auto tid = (pid_t) syscall(SYS_gettid);

        if (mode == CgroupMode::cg_off) {
            std::cout << "Not adding thread id " << tid << " cgroup support off" << std::endl;
            return;
        }

        // Get lock and add to controllers
        std::lock_guard<std::mutex> guard(groupMutex);

        this->addThread(tid, CG_CPU);
        this->addThread(tid, CG_NET_CLS);
    }

    void CGroup::addThread(pid_t threadId, const std::string &controller) {
        boost::filesystem::path tasksPath = this->getPathToFile(controller, "tasks");

        std::ofstream outfile;
        outfile.open(tasksPath.string(), std::ios_base::app);
        outfile << threadId << std::endl;
        outfile.flush();

        std::cout << "Added thread id " << threadId << " to " << controller << ":" << this->name << " at " << tasksPath
                  << std::endl;
    }

    boost::filesystem::path CGroup::getPathToFile(const std::string &controller, const std::string &file) {
        boost::filesystem::path path(BASE_DIR);
        path.append(controller);
        path.append(BASE_CGROUP_NAME);
        path.append(this->name);

        path.append(file);

        return path;
    }
}
