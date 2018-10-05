#include <catch/catch.hpp>

#include <worker/worker.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <syscall.h>
#include <sys/types.h>

using namespace worker;

namespace tests {

    TEST_CASE("Test basic cgroup properties", "[worker]") {
        util::setEnvVar("CGROUP_MODE", "on");

        CGroup cgA("foo");

        util::setEnvVar("CGROUP_MODE", "off");

        CGroup cgB("bar");

        REQUIRE(cgA.getMode() == CgroupMode::cg_on);
        REQUIRE(cgA.getName() == "foo");

        REQUIRE(cgB.getMode() == CgroupMode::cg_off);
        REQUIRE(cgB.getName() == "bar");

        util::unsetEnvVar("CGROUP_MODE");
    }

    void checkAddingToController(const std::string &controllerName) {
        CGroup cg("faasm1");

        boost::filesystem::path tasksPath("/sys/fs/cgroup");
        tasksPath.append(controllerName);
        tasksPath.append("faasm/faasm1/tasks");

        std::string fileBefore = util::readFileToString(tasksPath.string());

        REQUIRE(fileBefore.empty());

        cg.addCurrentThread();
        auto tid = (pid_t) syscall(SYS_gettid);

        std::string fileAfter = util::readFileToString(tasksPath.string());
        REQUIRE(boost::trim_copy(fileAfter) == std::to_string(tid));
    }

    TEST_CASE("Test adding thread to cpu controller", "[worker]") {
        checkAddingToController("cpu");
    }
}
