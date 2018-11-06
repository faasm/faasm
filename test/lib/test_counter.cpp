#include <catch/catch.hpp>

#include <infra/infra.h>

#include "faasm/memory.h"
#include "faasm/counter.h"

using namespace faasm;

namespace tests {
    TEST_CASE("Test normal counter operation", "[counter]") {
        infra::Redis redis;
        redis.flushAll();

        FaasmMemory mem;

        const char* key = "test_counter";
        initCounter(&mem, key);

        REQUIRE(getCounter(&mem, key) == 0);

        incrementCounter(&mem, key);
        REQUIRE(getCounter(&mem, key) == 1);

        incrementCounter(&mem, key);
        REQUIRE(getCounter(&mem, key) == 2);

        initCounter(&mem, key);
        REQUIRE(getCounter(&mem, key) == 0);

        incrementCounter(&mem, key);
        REQUIRE(getCounter(&mem, key) == 1);
    }

    TEST_CASE("Test uninitialised counter", "[counter]") {
        infra::Redis redis;
        redis.flushAll();

        FaasmMemory mem;

        const char* key = "test_uninit_key";
        REQUIRE(getCounter(&mem, key) == 0);

        incrementCounter(&mem, key);
        REQUIRE(getCounter(&mem, key) == 1);
    }
}