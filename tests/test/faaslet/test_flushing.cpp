#include <catch2/catch.hpp>

#include "faabric/scheduler/Scheduler.h"
#include "utils.h"

#include <faaslet/FaasletPool.h>
#include <boost/filesystem.hpp>
#include <faabric/executor/FaabricMain.h>
#include <faabric/util/config.h>
#include <faabric/util/files.h>
#include <module_cache/WasmModuleCache.h>

namespace tests {
TEST_CASE("Test flushing empty faaslet", "[faaslet]")
{
    faaslet::Faaslet f(0);
    f.flush();
}

TEST_CASE("Test flushing worker clears state", "[faaslet]")
{
    // Set up some state
    faabric::state::State& state = faabric::state::getGlobalState();
    state.getKV("demo", "blah", 10);
    state.getKV("other", "foo", 30);

    REQUIRE(state.getKVCount() == 2);

    faaslet::Faaslet f(0);
    f.flush();

    REQUIRE(state.getKVCount() == 0);
}

TEST_CASE("Test flushing worker clears shared files", "[faaslet]")
{
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();

    std::string relativePath = "flush-test.txt";
    boost::filesystem::path sharedPath(conf.sharedFilesDir);
    sharedPath.append(relativePath);

    if (boost::filesystem::exists(sharedPath)) {
        boost::filesystem::remove(sharedPath);
    }
    boost::filesystem::create_directories(conf.sharedFilesDir);

    // Enter some data
    std::vector<uint8_t> bytes = { 0, 1, 2, 3 };
    faabric::util::writeBytesToFile(sharedPath.string(), bytes);
    REQUIRE(boost::filesystem::exists(sharedPath));

    // Flush and check file is gone
    faaslet::Faaslet f(0);
    f.flush();
    REQUIRE(!boost::filesystem::exists(sharedPath));
}

TEST_CASE("Test flushing worker clears zygotes", "[faaslet]")
{
    const faabric::Message msgA = faabric::util::messageFactory("demo", "echo");
    const faabric::Message msgB =
      faabric::util::messageFactory("demo", "dummy");

    module_cache::WasmModuleCache& reg = module_cache::getWasmModuleCache();
    reg.getCachedModule(msgA);
    reg.getCachedModule(msgB);

    REQUIRE(reg.getTotalCachedModuleCount() == 2);

    faaslet::Faaslet f(0);
    f.flush();
    REQUIRE(reg.getTotalCachedModuleCount() == 0);
}

TEST_CASE("Test flushing worker clears scheduler", "[faaslet]")
{
    faabric::Message msgA = faabric::util::messageFactory("demo", "echo");
    faabric::Message msgB = faabric::util::messageFactory("demo", "dummy");

    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();
    sch.callFunction(msgA);
    sch.callFunction(msgB);

    REQUIRE(sch.getFunctionInFlightCount(msgA) == 1);
    REQUIRE(sch.getFunctionInFlightCount(msgB) == 1);

    faaslet::Faaslet f(0);
    f.flush();

    REQUIRE(sch.getFunctionInFlightCount(msgA) == 0);
    REQUIRE(sch.getFunctionInFlightCount(msgB) == 0);
}
}
