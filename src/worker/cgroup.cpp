#include "worker/worker.h"

#include <sys/stat.h>
#include <thread>
#include <sstream>
#include <util/util.h>

namespace worker {
    static const std::string BASE_DIR = "/sys/fs/cgroup/";

    CGroup::CGroup(const std::string &name) :
            name(name) {

        // Get which cgroup mode we're operating in
        std::string modeEnv = util::getEnvVar("CGROUP_MODE", "v1");

        if (modeEnv == "v1") {
            mode = CgroupMode::v1;
        } else if (modeEnv == "v2") {
            mode = CgroupMode::v2;
        } else {
            mode = CgroupMode::off;
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
        if (mode == CgroupMode::off) {
            std::cout << "Not limiting CPU, cgroup support off" << std::endl;
            return;
        }

        this->mkdirForController("cpu");
        this->controllers.emplace_back("cpu");
    }

    void CGroup::addCurrentThread() {
        std::thread::id thisThreadId = std::this_thread::get_id();

        if (mode == CgroupMode::off) {
            std::cout << "Not adding thread id " << thisThreadId << " cgroup support off" << std::endl;
            return;
        }

        // Add the thread id to the tasks files for this group
        for (auto &ctrl : this->controllers) {
            boost::filesystem::path tasksPath = this->getPathToFile(ctrl, "tasks");

            // Convert ID to string
            std::stringstream ss;
            ss << thisThreadId;

            // Get lock and write to file
            std::lock_guard<std::mutex> guard(this->tasksMutex);
            util::appendToFile(tasksPath.string(), ss.str());

            std::cout << "Added thread id " << thisThreadId << " to cgroup " << ctrl << ":" << this->name << std::endl;
        }
    }
}
