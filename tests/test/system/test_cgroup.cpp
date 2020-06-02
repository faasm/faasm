#include <catch/catch.hpp>

#include <util/environment.h>
#include <util/files.h>
#include <util/config.h>
#include <util/string_tools.h>

#include <system/CGroup.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <thread>
#include <syscall.h>

using namespace isolation;

namespace tests {
    static int cgroupCheckSuccessful = 0;

    TEST_CASE("Test cgroup on/ off", "[faaslet]") {
        util::SystemConfig &conf = util::getSystemConfig();
        std::string original = conf.cgroupMode;

        // Ignore this test in CI, not able to run privileged hence can't create cgroups
        if(conf.hostType == "ci") {
            return;
        }

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
        util::setEnvVar("CGROUP_MODE", original);
        conf.reset();
    }

    // Need to run this check in a separate thread
    void checkCgroupAddition() {
        boost::filesystem::path cgroupPath("/sys/fs/cgroup");
        cgroupPath.append("cpu");
        cgroupPath.append(BASE_CGROUP_NAME);

        REQUIRE(boost::filesystem::exists(cgroupPath));

        // Add the current thread
        // Note - it's possible that another test has already done this, so
        // really this test should remove the thread beforehand, however,
        // you need to be root to do this.
        CGroup cg(BASE_CGROUP_NAME);
        cg.addCurrentThread();

        // Check this thread is in the cgroup
        cgroupPath.append("tasks");
        std::string tid = std::to_string((pid_t) syscall(SYS_gettid));
        std::string fileContents = util::readFileToString(cgroupPath.string());

        REQUIRE(util::contains(fileContents, tid));

        // Set success
        cgroupCheckSuccessful = 1;
    }

    TEST_CASE("Test adding thread to cpu controller", "[faaslet]") {
        util::SystemConfig &conf = util::getSystemConfig();

        // Ignore this test in CI, not able to run privileged hence can't create cgroups
        if(conf.hostType == "ci") {
            return;
        }

        REQUIRE(conf.cgroupMode == "on");

        std::thread t(checkCgroupAddition);
        t.join();

        REQUIRE(cgroupCheckSuccessful == 1);
    }
}
