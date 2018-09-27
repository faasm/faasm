#include "worker/worker.h"

#include <sys/stat.h>
#include <thread>
#include <sstream>
#include <util/util.h>

namespace worker {
    static const std::string BASE_DIR = "/sys/fs/cgroup/";

    static std::mutex tasksMutex;

    CGroup::CGroup(const std::string &name) :
            name(name) {

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

    void CGroup::limitCpu() {
        if (mode == CgroupMode::cg_off) {
            std::cout << "Not limiting CPU, cgroup support off" << std::endl;
            return;
        }

        this->mkdirForController("cpu");
        this->controllers.emplace_back("cpu");
    }

    void CGroup::addThread(pid_t threadId) {
        if (mode == CgroupMode::cg_off) {
            std::cout << "Not adding thread id " << threadId << " cgroup support off" << std::endl;
            return;
        }

        // Add for each controller
        for (auto &ctrl : this->controllers) {
            boost::filesystem::path tasksPath = this->getPathToFile(ctrl, "tasks");

            // Get lock and write to file
            std::lock_guard<std::mutex> guard(tasksMutex);

            std::ofstream outfile;
            outfile.open(tasksPath.string(), std::ios_base::app);
            outfile << threadId << std::endl;
            outfile.flush();

            std::cout << "Added thread id " << threadId << " to " << ctrl << ":" << this->name << " at " << tasksPath << std::endl;
        }
    }
}
