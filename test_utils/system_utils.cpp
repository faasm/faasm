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
#include <worker/worker.h>


namespace tests {
    void cleanSystem() {
        util::SystemConfig &conf = util::getSystemConfig();

        // Clear out state
        redis::Redis::getState().flushAll();
        redis::Redis::getQueue().flushAll();

        // Flush worker stuff
        worker::flushWorkerHost();

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