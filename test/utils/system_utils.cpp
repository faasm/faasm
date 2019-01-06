#include <catch/catch.hpp>

#include "utils.h"

namespace tests {
    void cleanSystem() {
        redisState.flushAll();
        redisQueue.flushAll();

        workerThreadPool.reset();
    }
}