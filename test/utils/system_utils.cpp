#include <catch/catch.hpp>

#include "utils.h"

#include <redis/Redis.h>
#include <state/State.h>
#include <scheduler/Scheduler.h>
#include <emulator/emulator.h>


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

        // Reset system config
        util::getSystemConfig().reset();

        // Set emulator user
        unsetEmulatorUser();
        setEmulatorUser("tester");
    }
}