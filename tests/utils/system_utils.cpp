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
#include <storage/SharedFilesManager.h>
#include <boost/filesystem.hpp>
#include <worker/worker.h>


namespace tests {
    void cleanSystem() {
        util::SystemConfig &conf = util::getSystemConfig();

        // Clear out state
        redis::Redis::getState().flushAll();
        redis::Redis::getQueue().flushAll();

        // Clear out any cached state
        state::getGlobalState().forceClearAll();

        // Clear shared files
        storage::getSharedFilesManager().clear();

        // Nuke shared files
        boost::filesystem::remove_all(conf.sharedFilesDir);

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

        // When running tests we don't get the benefit of thread-local storage
        // As everything is single-threaded, therefore we need to clear out the
        // open file descriptors
        wasm::WasmModule m;
        m.clearFds();

        // Set emulator user
        resetEmulator();
        setEmulatorUser("tester");
    }
}