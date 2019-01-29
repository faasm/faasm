#include <catch/catch.hpp>

#include "utils.h"

namespace tests {
    void cleanSystem() {
        redis::Redis::getState().flushAll();
        redis::Redis::getQueue().flushAll();

        state::getGlobalState().forceClearAll();

        scheduler::getScheduler().reset();

        workerThreadPool.reset();
    }
}