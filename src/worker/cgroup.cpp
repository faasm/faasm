#include "worker/worker.h"

#include <sys/stat.h>
#include <thread>
#include <sstream>
#include <util/util.h>

namespace worker {
    static const std::string BASE_DIR = "/sys/fs/cgroup/";
    static std::string TOP_LEVEL_CGROUP = "faasm";

    static std::mutex tasksMutex;

    CGroup::CGroup() {
        name = TOP_LEVEL_CGROUP;

        // Get which cgroup mode we're operating in
        std::string modeEnv = util::getEnvVar("CGROUP_MODE", "on");

        if (modeEnv == "on") {
            mode = CgroupMode::cg_on;
        } else {
            mode = CgroupMode::cg_off;
        }
    }

    boost::filesystem::path CGroup::getPathToController(const std::string &controller) {
        boost::filesystem::path controllerPath(BASE_DIR);
        controllerPath.append(controller);
        controllerPath.append(this->name);

        return controllerPath;
    }

    boost::filesystem::path CGroup::getPathToFile(const std::string &controller, const std::string &file) {
        boost::filesystem::path filePath = this->getPathToController(controller);
        filePath.append(file);

        return filePath;
    }

    /** Creates directory for controller if doesn't exist */
    void CGroup::mkdirForController(const std::string &controller) {
        boost::filesystem::path controllerPath = this->getPathToController(controller);
        boost::filesystem::create_directory(controllerPath);
    }

    void CGroup::addController(const std::string &controller) {
        if (mode == CgroupMode::cg_off) {
            std::cout << "Not limiting " << controller << " cgroup support off" << std::endl;
            return;
        }

        this->mkdirForController(controller);
    }

    void CGroup::addThread(pid_t threadId, const std::string &controller) {
        if (mode == CgroupMode::cg_off) {
            std::cout << "Not adding thread id " << threadId << " cgroup support off" << std::endl;
            return;
        }

        boost::filesystem::path tasksPath = this->getPathToFile(controller, "tasks");

        // Get lock and write to file
        std::lock_guard<std::mutex> guard(tasksMutex);

        std::ofstream outfile;
        outfile.open(tasksPath.string(), std::ios_base::app);
        outfile << threadId << std::endl;
        outfile.flush();

        std::cout << "Added thread id " << threadId << " to " << controller << ":" << this->name << " at " << tasksPath << std::endl;
    }
}
