#include <catch/catch.hpp>

#include "utils.h"

#include <redis/Redis.h>
#include <state/State.h>
#include <scheduler/Scheduler.h>


namespace tests {
    void cleanSystem() {
        redis::Redis::getState().flushAll();
        redis::Redis::getQueue().flushAll();

        state::getGlobalState().forceClearAll();

        scheduler::getScheduler().clear();

        workerThreadPool.reset();
    }
}