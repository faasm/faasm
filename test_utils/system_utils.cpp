#include <catch/catch.hpp>

extern "C" {
#include <emulator/emulator_api.h>
}

#include "utils.h"

#include <redis/Redis.h>
#include <state/State.h>
#include <scheduler/Scheduler.h>
#include <emulator/emulator.h>
#include <memory/MemorySnapshotRegister.h>
#include <zygote/ZygoteRegistry.h>
#include <storage/SharedFilesManager.h>
#include <boost/filesystem.hpp>


namespace tests {
    void cleanSystem() {
        util::SystemConfig &conf = util::getSystemConfig();

        // Clear out state
        redis::Redis::getState().flushAll();
        redis::Redis::getQueue().flushAll();
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

        // Clear memory snapshots
        memory::getGlobalMemorySnapshotRegister().clear();

        // Clear zygotes
        zygote::getZygoteRegistry().clear();

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