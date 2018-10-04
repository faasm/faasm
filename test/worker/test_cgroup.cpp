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

        boost::filesystem::path cpuTasksPath("/sys/fs/cgroup/cpu/faasm/faasm1/tasks");
        boost::filesystem::path netClsTasksPath("/sys/fs/net_cls/cpu/faasm/faasm1/tasks");

        std::string cpuFileBefore = util::readFileToString(cpuTasksPath.string());
        std::string netClsFileBefore = util::readFileToString(netClsTasksPath.string());

        REQUIRE(cpuFileBefore.empty());
        REQUIRE(netClsFileBefore.empty());

        cg.addCurrentThread();

        auto tid = (pid_t) syscall(SYS_gettid);

        std::string cpuFileAfter = util::readFileToString(cpuTasksPath.string());
        std::string netClsFileAfter = util::readFileToString(netClsTasksPath.string());

        REQUIRE(boost::trim_copy(cpuFileAfter) == std::to_string(tid));
        REQUIRE(boost::trim_copy(netClsFileAfter) == std::to_string(tid));
    }
}
