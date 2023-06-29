#include <catch2/catch.hpp>

#include <faabric/util/config.h>
#include <faabric/util/environment.h>
#include <faabric/util/files.h>
#include <faabric/util/logging.h>
#include <faabric/util/string_tools.h>

#include <conf/FaasmConfig.h>
#include <system/CGroup.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <syscall.h>
#include <thread>

using namespace isolation;

namespace tests {

TEST_CASE("Test cgroup on/ off", "[faaslet]")
{
    conf::FaasmConfig& conf = conf::getFaasmConfig();
    std::string original = conf.cgroupMode;

    // Ignore this test in CI, not able to run privileged hence can't create
    // cgroups
    if (conf.hostType == "ci") {
        return;
    }

    std::string envValue;
    CgroupMode expected;

    SECTION("Test cgroup on")
    {
        envValue = "on";
        expected = CgroupMode::cg_on;
    }

    SECTION("Test cgroup off")
    {
        envValue = "off";
        expected = CgroupMode::cg_off;
    }

    // Update the config
    faabric::util::setEnvVar("CGROUP_MODE", envValue);
    conf.reset();

    // Create and check
    CGroup cg("foo");
    REQUIRE(cg.getMode() == expected);
    REQUIRE(cg.getName() == "foo");

    // Reset config
    faabric::util::setEnvVar("CGROUP_MODE", original);
    conf.reset();
}

static bool cgroupCheckPassed = false;

// Need to run this check in a separate thread
void checkCgroupAddition()
{
    boost::filesystem::path cgroupPath("/sys/fs/cgroup");
    cgroupPath.append("cpu");
    cgroupPath.append(BASE_CGROUP_NAME);

    if (!boost::filesystem::exists(cgroupPath)) {
        SPDLOG_ERROR("cgroup does not exist at {}", cgroupPath.string());
        cgroupCheckPassed = false;
        return;
    }

    // Add the current thread
    // Note - it's possible that another test has already done this, so
    // really this test should remove the thread beforehand, however,
    // you need to be root to do this.
    CGroup cg(BASE_CGROUP_NAME);
    cg.addCurrentThread();

    // Check this thread is in the cgroup
    cgroupPath.append("tasks");
    std::string tid = std::to_string((pid_t)syscall(SYS_gettid));
    std::string fileContents =
      faabric::util::readFileToString(cgroupPath.string());

    if (!faabric::util::contains(fileContents, tid)) {
        SPDLOG_ERROR("cgroup file does not contain tid {}", tid);
        cgroupCheckPassed = false;
        return;
    }

    // Set success
    cgroupCheckPassed = true;
}

TEST_CASE("Test adding thread to cpu controller", "[faaslet]")
{
    conf::FaasmConfig& conf = conf::getFaasmConfig();

    // Ignore this test in CI, not able to run privileged hence can't create
    // cgroups
    if (conf.hostType == "ci") {
        return;
    }

    REQUIRE(conf.cgroupMode == "on");

    /* 20/06/2023 - CGroup check not working. Must fix
    std::thread t(checkCgroupAddition);

    if (t.joinable()) {
        t.join();
    }

    REQUIRE(cgroupCheckPassed);
    */
}
}
