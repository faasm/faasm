#include <catch/catch.hpp>

#include <util/environment.h>
#include <util/files.h>
#include <util/config.h>

#include <worker/CGroup.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <syscall.h>
#include <sys/types.h>

using namespace worker;

namespace tests {
    TEST_CASE("Test cgroup on/ off", "[worker]") {
        util::SystemConfig &conf = util::getSystemConfig();

        std::string envValue;
        CgroupMode  expected;

        SECTION("Test cgroup on") {
            envValue = "on";
            expected = CgroupMode::cg_on;
        }

        SECTION("Test cgroup off") {
            envValue = "off";
            expected = CgroupMode::cg_off;
        }

        // Update the config
        util::setEnvVar("CGROUP_MODE", envValue);
        conf.reset();

        // Create and check
        CGroup cg("foo");
        REQUIRE(cg.getMode() == expected);
        REQUIRE(cg.getName() == "foo");

        // Reset config
        util::unsetEnvVar("CGROUP_MODE");
        conf.reset();
    }

    TEST_CASE("Test adding thread to cpu controller", "[worker]") {
        util::SystemConfig &conf = util::getSystemConfig();
        REQUIRE(conf.cgroupMode == "on");

        // Delete the cgroup if it exists already
        boost::filesystem::path cgroupPath("/sys/fs/cgroup");
        cgroupPath.append("cpu");
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
}
