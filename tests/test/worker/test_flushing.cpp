#include <catch/catch.hpp>

#include "utils.h"

#include <worker/worker.h>
#include <util/config.h>
#include <boost/filesystem.hpp>
#include <util/files.h>
#include <module_cache/WasmModuleCache.h>

namespace tests {
    TEST_CASE("Test flushing empty worker", "[worker]") {
        worker::flushWorkerHost();
    }

    TEST_CASE("Test flushing worker clears state", "[worker]") {
        // Set up some state
        state::State &state = state::getGlobalState();
        state.getKV("demo", "blah", 10);
        state.getKV("other", "foo", 30);

        REQUIRE(state.getKVCount() == 2);

        worker::flushWorkerHost();

        REQUIRE(state.getKVCount() == 0);
    }

    TEST_CASE("Test flushing worker clears shared files", "[worker]") {
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
        worker::flushWorkerHost();
        REQUIRE(!boost::filesystem::exists(sharedPath));
    }

    TEST_CASE("Test flushing worker clears zygotes", "[worker]") {
        const message::Message msgA = util::messageFactory("demo", "echo");
        const message::Message msgB = util::messageFactory("demo", "dummy");

        module_cache::WasmModuleCache &reg = module_cache::getWasmModuleCache();
        reg.getCachedModule(msgA);
        reg.getCachedModule(msgB);

        REQUIRE(reg.getTotalCachedModuleCount() == 2);

        worker::flushWorkerHost();
        REQUIRE(reg.getTotalCachedModuleCount() == 0);
    }
    
    TEST_CASE("Test flushing worker clears scheduler", "[worker]") {
        message::Message msgA = util::messageFactory("demo", "echo");
        message::Message msgB = util::messageFactory("demo", "dummy");

        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.callFunction(msgA);
        sch.callFunction(msgB);

        REQUIRE(sch.getFunctionInFlightCount(msgA) == 1);
        REQUIRE(sch.getFunctionInFlightCount(msgB) == 1);

        worker::flushWorkerHost();

        REQUIRE(sch.getFunctionInFlightCount(msgA) == 0);
        REQUIRE(sch.getFunctionInFlightCount(msgB) == 0);
    }
}