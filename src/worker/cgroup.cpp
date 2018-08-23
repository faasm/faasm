#include "worker/worker.h"

#include <sys/stat.h>
#include <unistd.h>

#include <util/util.h>

namespace worker {
    static const std::string BASE_DIR = "/sys/fs/cgroup/";

    CGroup::CGroup(const std::string &name) :
            name(name) {

        // Get which cgroup mode we're operating in
        mode = util::getEnvVar("CGROUP_MODE", "off");
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
        if(mode == "off") {
            std::cout << "Not limiting CPU, cgroup support off" << std::endl;
            return;
        }

        this->mkdirForController("cpu");
        this->controllers.emplace_back("cpu");
    }

    void CGroup::addCurrentPid() {
        pid_t thisPid = getpid();

        if(mode == "off") {
            std::cout << "Not adding pid " << thisPid << " cgroup support off" << std::endl;
            return;
        }

        // Add the pid to the tasks file for this group
        for(auto &ctrl : this->controllers) {
            boost::filesystem::path tasksPath = this->getPathToFile(ctrl, "tasks");
            util::appendToFile(tasksPath.string(), std::to_string(thisPid));

            std::cout << "Added PID " << thisPid << " to cgroup " << ctrl << ":" << this->name << std::endl;
        }
    }
}
