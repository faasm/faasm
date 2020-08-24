#include <catch/catch.hpp>

#include "utils.h"

#include <faaslet/FaasmMain.h>
#include <util/config.h>
#include <boost/filesystem.hpp>
#include <util/files.h>
#include <module_cache/WasmModuleCache.h>

namespace tests {
    TEST_CASE("Test flushing empty worker", "[faaslet]") {
        faaslet::flushFaasletHost();
    }

    TEST_CASE("Test flushing worker clears state", "[faaslet]") {
        // Set up some state
        state::State &state = state::getGlobalState();
        state.getKV("demo", "blah", 10);
        state.getKV("other", "foo", 30);

        REQUIRE(state.getKVCount() == 2);

        faaslet::flushFaasletHost();

        REQUIRE(state.getKVCount() == 0);
    }

    TEST_CASE("Test flushing worker clears shared files", "[faaslet]") {
        util::SystemConfig &conf = util::getSystemConfig();

        std::string relativePath = "flush-test.txt";
        boost::filesystem::path sharedPath(conf.sharedFilesDir);
        sharedPath.append(relativePath);

        if (boost::filesystem::exists(sharedPath)) {
            boost::filesystem::remove(sharedPath);
        }
        boost::filesystem::create_directories(conf.sharedFilesDir);

        // Enter some data
        std::vector<uint8_t> bytes = {0, 1, 2, 3};
        util::writeBytesToFile(sharedPath.string(), bytes);
        REQUIRE(boost::filesystem::exists(sharedPath));

        // Flush and check file is gone
        faaslet::flushFaasletHost();
        REQUIRE(!boost::filesystem::exists(sharedPath));
    }

    TEST_CASE("Test flushing worker clears zygotes", "[faaslet]") {
        const faabric::Message msgA = util::messageFactory("demo", "echo");
        const faabric::Message msgB = util::messageFactory("demo", "dummy");

        module_cache::WasmModuleCache &reg = module_cache::getWasmModuleCache();
        reg.getCachedModule(msgA);
        reg.getCachedModule(msgB);

        REQUIRE(reg.getTotalCachedModuleCount() == 2);

        faaslet::flushFaasletHost();
        REQUIRE(reg.getTotalCachedModuleCount() == 0);
    }
    
    TEST_CASE("Test flushing worker clears scheduler", "[faaslet]") {
        faabric::Message msgA = util::messageFactory("demo", "echo");
        faabric::Message msgB = util::messageFactory("demo", "dummy");

        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.callFunction(msgA);
        sch.callFunction(msgB);

        REQUIRE(sch.getFunctionInFlightCount(msgA) == 1);
        REQUIRE(sch.getFunctionInFlightCount(msgB) == 1);

        faaslet::flushFaasletHost();

        REQUIRE(sch.getFunctionInFlightCount(msgA) == 0);
        REQUIRE(sch.getFunctionInFlightCount(msgB) == 0);
    }
}