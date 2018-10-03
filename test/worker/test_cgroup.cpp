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

    TEST_CASE("Test adding thread to cpu", "[worker]") {
        CGroup cg("faasm1");

        boost::filesystem::path cpuPath("/sys/fs/cgroup/cpu/faasm/faasm1/tasks");
        std::string fileContents = util::readFileToString(cpuPath.string());

        REQUIRE(fileContents.empty());

        cg.addCurrentThread();

        std::string fileContentAfter = util::readFileToString(cpuPath.string());
        std::string trimmedContentAfter = boost::trim_copy(fileContentAfter);

        auto tid = (pid_t) syscall(SYS_gettid);
        REQUIRE(trimmedContentAfter == std::to_string(tid));
    }
}
