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

    TEST_CASE("Test redis enqueue/ dequeue", "[redis]") {
        redis::RedisClient cli;

        // Clear queue initially
        cli.flushall();

        // Enqueue some values
        std::string queueName = "my_queue";
        std::string valueA = "val a";
        std::string valueB = "val b";
        std::string valueC = "val c";

        cli.enqueue("my_queue", valueA);
        cli.enqueue("my_queue", valueB);
        cli.enqueue("my_queue", valueC);

        // Check expected length
        cli.llen(queueName, [](cpp_redis::reply &r) {
            int64_t actual = r.as_integer();

            REQUIRE(3 == actual);
        });

        // Now dequeue
        cli.dequeue(queueName, [valueA](const std::string &res){
            REQUIRE(valueA == res);
        });

        // Dequeue again
        cli.dequeue(queueName, [valueB](const std::string &res){
            REQUIRE(valueB == res);
        });

        cli.sync_commit();
    }

}