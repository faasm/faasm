#include <catch/catch.hpp>

#include "utils.h"

#include <redis/Redis.h>
#include <state/State.h>
#include <scheduler/Scheduler.h>
#include <emulator/emulator.h>
#include <memory/MemorySnapshotRegister.h>
#include <zygote/ZygoteRegistry.h>


namespace tests {
    void cleanSystem() {
        // Clear out state
        redis::Redis::getState().flushAll();
        redis::Redis::getQueue().flushAll();
        state::getGlobalState().forceClearAll();

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
        util::getSystemConfig().reset();

        // Set emulator user
        unsetEmulatorUser();
        setEmulatorUser("tester");
    }
}