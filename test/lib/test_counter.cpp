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

        FaasmMemory mem;

        const char *key = "test_counter";
        initCounter(&mem, key, false);

        REQUIRE(getCounter(&mem, key, false) == 0);

        incrementCounter(&mem, key, false);
        REQUIRE(getCounter(&mem, key, false) == 1);

        incrementCounter(&mem, key, false);
        REQUIRE(getCounter(&mem, key, false) == 2);

        initCounter(&mem, key, false);
        REQUIRE(getCounter(&mem, key, false) == 0);

        incrementCounter(&mem, key, false);
        REQUIRE(getCounter(&mem, key, false) == 1);
    }

    TEST_CASE("Test counter over big number", "[counter]") {
        redis::Redis &redisQueue = redis::Redis::getQueue();
        redisQueue.flushAll();
        state::getGlobalState().forceClearAll();

        FaasmMemory mem;

        const char *key = "test_counter";
        initCounter(&mem, key, false);

        for (int i = 0; i < 1000; i++) {
            incrementCounter(&mem, key, false);
        }

        REQUIRE(getCounter(&mem, key, false) == 1000);
    }

    TEST_CASE("Test uninitialised counter", "[counter]") {
        redis::Redis &redisQueue = redis::Redis::getQueue();
        redisQueue.flushAll();
        state::getGlobalState().forceClearAll();

        FaasmMemory mem;

        const char *key = "test_uninit_key";
        initCounter(&mem, key, false);
        REQUIRE(getCounter(&mem, key, false) == 0);

        incrementCounter(&mem, key, false);
        REQUIRE(getCounter(&mem, key, false) == 1);
    }
}