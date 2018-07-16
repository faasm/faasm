#include <catch/catch.hpp>
#include <redis/redis.h>
#include <string>

namespace tests {

    TEST_CASE("Test redis connection", "[redis]") {
        redis::RedisClient cli;

        std::string expected = "foobar";
        std::string actual = cli.check(expected);

        REQUIRE(expected == actual);
    }

}