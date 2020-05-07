#include <catch/catch.hpp>

extern "C" {
#include <emulator/emulator_api.h>
}

#include "utils.h"

#include <redis/Redis.h>
#include <state/State.h>
#include <scheduler/Scheduler.h>
#include <emulator/emulator.h>
#include <module_cache/WasmModuleCache.h>


namespace tests {
    void cleanSystem() {
        util::SystemConfig &conf = util::getSystemConfig();

        // Clear out state
        redis::Redis::getState().flushAll();
        redis::Redis::getQueue().flushAll();

        // Clear out any cached state, do so for both modes
        std::string &originalStateMode = conf.stateMode;
        conf.stateMode = "inmemory";
        state::getGlobalState().forceClearAll(true);
        conf.stateMode = "redis";
        state::getGlobalState().forceClearAll(true);
        conf.stateMode = originalStateMode;

        // Clear shared files
        storage::FileSystem::clearSharedFiles();

        // Reset scheduler
        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.clear();
        sch.addNodeToGlobalSet();

        // Clear out global message bus
        scheduler::getGlobalMessageBus().clear();

        // Clear zygotes
        module_cache::getWasmModuleCache().clear();

        // Reset system config
        conf.reset();

        // Set emulator user
        resetEmulator();
        setEmulatorUser("tester");
    }
}