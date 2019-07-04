#include <catch/catch.hpp>

#include <redis/Redis.h>
#include <state/State.h>

#include "faasm/memory.h"
#include "faasm/counter.h"
#include "utils.h"

using namespace faasm;

namespace tests {
    TEST_CASE("Test normal counter operation", "[counter]") {
        redis::Redis &redisQueue = redis::Redis::getQueue();
        redisQueue.flushAll();
        state::getGlobalState().forceClearAll();

        const char *key = "test_counter";
        faasmInitCounter(key, false);

        REQUIRE(getCounter(key, false) == 0);

        incrementCounter(key, false);
        REQUIRE(getCounter(key, false) == 1);

        incrementCounter(key, false);
        REQUIRE(getCounter(key, false) == 2);

        faasmInitCounter(key, false);
        REQUIRE(getCounter(key, false) == 0);

        incrementCounter(key, false);
        REQUIRE(getCounter(key, false) == 1);
    }

    TEST_CASE("Test counter over big number", "[counter]") {
        redis::Redis &redisQueue = redis::Redis::getQueue();
        redisQueue.flushAll();
        state::getGlobalState().forceClearAll();

        const char *key = "test_counter";
        faasmInitCounter(key, false);

        for (int i = 0; i < 1000; i++) {
            incrementCounter(key, false);
        }

        REQUIRE(getCounter(key, false) == 1000);
    }

    TEST_CASE("Test uninitialised counter", "[counter]") {
        redis::Redis &redisQueue = redis::Redis::getQueue();
        redisQueue.flushAll();
        state::getGlobalState().forceClearAll();

        const char *key = "test_uninit_key";
        faasmInitCounter(key, false);
        REQUIRE(getCounter(key, false) == 0);

        incrementCounter(key, false);
        REQUIRE(getCounter(key, false) == 1);
    }
}