#include <catch/catch.hpp>

#include <util/environment.h>
#include <util/files.h>

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
        // Delete the cgroup if it exists already
        boost::filesystem::path cgroupPath("/sys/fs/cgroup");
        cgroupPath.append(controllerName);
        cgroupPath.append(BASE_CGROUP_NAME);

        REQUIRE(boost::filesystem::exists(cgroupPath));

        // Check tasks file is empty
        cgroupPath.append("tasks");

        std::string fileBefore = util::readFileToString(cgroupPath.string());

        REQUIRE(fileBefore.empty());

        // Add the current thread
        CGroup cg(BASE_CGROUP_NAME);
        cg.addCurrentThread();
        auto tid = (pid_t) syscall(SYS_gettid);

        std::string fileAfter = util::readFileToString(cgroupPath.string());
        REQUIRE(boost::trim_copy(fileAfter) == std::to_string(tid));
    }

    TEST_CASE("Test adding thread to cpu controller", "[worker]") {
        checkAddingToController("cpu");
    }
}
