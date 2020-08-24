#include <catch/catch.hpp>

#include "faabric_utils.h"

#include <faabric/redis/Redis.h>
#include <faabric/state/State.h>
#include <faabric/scheduler/Scheduler.h>
#include <faabric/scheduler/MpiWorldRegistry.h>


namespace tests {
    void cleanFaabric() {
        faabric::utilSystemConfig &conf = faabric::utilgetSystemConfig();

        // Clear out Redis
        redis::Redis::getState().flushAll();
        redis::Redis::getQueue().flushAll();

        // Clear out any cached state, do so for both modes
        std::string &originalStateMode = conf.stateMode;
        conf.stateMode = "inmemory";
        state::getGlobalState().forceClearAll(true);
        conf.stateMode = "redis";
        state::getGlobalState().forceClearAll(true);
        conf.stateMode = originalStateMode;

        // Reset scheduler
        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.clear();
        sch.addHostToGlobalSet();

        // Reset system config
        conf.reset();

        // Clear out MPI worlds
        scheduler::MpiWorldRegistry &mpiRegistry = scheduler::getMpiWorldRegistry();
        mpiRegistry.clear();
    }
}