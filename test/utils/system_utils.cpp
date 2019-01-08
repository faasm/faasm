#include <catch/catch.hpp>

#include "utils.h"

namespace tests {
    void cleanSystem() {
        infra::Redis::getState().flushAll();
        infra::Redis::getQueue().flushAll();

        state::getGlobalState().forceClearAll();

        workerThreadPool.reset();
    }
}