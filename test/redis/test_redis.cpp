#include <catch/catch.hpp>

#include "utils.h"

#include <redis/Redis.h>
#include <util/bytes.h>

#include <algorithm>

using namespace redis;

namespace tests {

    TEST_CASE("Basic Redis operations", "[redis]") {
        Redis &redisQueue = Redis::getQueue();
        Redis &redisState = Redis::getState();
        redisQueue.flushAll();

        const std::string QUEUE_NAME = "my queue";

        const std::string VALUE_A = "val a";
        const std::string VALUE_B = "val b";
        const std::string VALUE_C = "val c";

        const std::vector<uint8_t> BYTES_A = util::stringToBytes(VALUE_A);
        const std::vector<uint8_t> BYTES_B = util::stringToBytes(VALUE_B);
        const std::vector<uint8_t> BYTES_C = util::stringToBytes(VALUE_C);

        SECTION("Test ping") {
            // Will throw exception if something is wrong
            redisState.ping();
            redisQueue.ping();
        }
        
        SECTION("Test incr/ decr") {
            std::string key = "test_counter";

            REQUIRE(redisQueue.getCounter(key) == 0);

            REQUIRE(redisQueue.incr(key) == 1);
            REQUIRE(redisQueue.getCounter(key) == 1);

            REQUIRE(redisQueue.incr(key) == 2);
            REQUIRE(redisQueue.incr(key) == 3);
            REQUIRE(redisQueue.incr(key) == 4);
            REQUIRE(redisQueue.getCounter(key) == 4);

            REQUIRE(redisQueue.decr(key) == 3);
            REQUIRE(redisQueue.decr(key) == 2);
            REQUIRE(redisQueue.getCounter(key) == 2);
        }

        SECTION("Test enqueue/ dequeue bytes") {
            // Enqueue some values
            std::vector<uint8_t> bytesA = util::stringToBytes(VALUE_A);
            redisQueue.enqueueBytes(QUEUE_NAME, BYTES_A);
            redisQueue.enqueueBytes(QUEUE_NAME, BYTES_B);
            redisQueue.enqueueBytes(QUEUE_NAME, BYTES_C);

            // Check expected length
            REQUIRE(redisQueue.listLength(QUEUE_NAME) == 3);

            // Blocking dequeue
            std::vector<uint8_t> actual = redisQueue.dequeueBytes(QUEUE_NAME);
            REQUIRE(BYTES_A == actual);
            REQUIRE(redisQueue.listLength(QUEUE_NAME) == 2);

            // Dequeue again
            std::vector<uint8_t> actual2 = redisQueue.dequeueBytes(QUEUE_NAME);
            REQUIRE(BYTES_B == actual2);
            REQUIRE(redisQueue.listLength(QUEUE_NAME) == 1);
        }

        SECTION("Test enqueue/ dequeue strings") {
            // Enqueue some values
            redisQueue.enqueue(QUEUE_NAME, VALUE_A);
            redisQueue.enqueue(QUEUE_NAME, VALUE_B);
            redisQueue.enqueue(QUEUE_NAME, VALUE_C);

            // Check expected length
            REQUIRE(redisQueue.listLength(QUEUE_NAME) == 3);

            // Blocking dequeue
            std::string actual = redisQueue.dequeue(QUEUE_NAME);
            REQUIRE(VALUE_A == actual);
            REQUIRE(redisQueue.listLength(QUEUE_NAME) == 2);

            // Dequeue again
            std::string actual2 = redisQueue.dequeue(QUEUE_NAME);
            REQUIRE(VALUE_B == actual2);
            REQUIRE(redisQueue.listLength(QUEUE_NAME) == 1);
        }

        SECTION("Test set/ get/ del") {
            std::string keyA = "key a";
            std::string keyB = "key b";

            redisQueue.set(keyA, BYTES_A);
            redisQueue.set(keyB, BYTES_B);

            // Test multiple gets on the same key
            std::vector<uint8_t> actualA1 = redisQueue.get(keyA);
            std::vector<uint8_t> actualA2 = redisQueue.get(keyA);
            std::vector<uint8_t> actualB = redisQueue.get(keyB);

            REQUIRE(BYTES_A == actualA1);
            REQUIRE(BYTES_A == actualA2);

            REQUIRE(BYTES_B == actualB);

            // Check deleting
            redisQueue.del(keyA);
            std::vector<uint8_t> actualDel = redisQueue.get(keyA);
            REQUIRE(actualDel.empty());

            // Check other still there
            REQUIRE(redisQueue.get(keyB) == BYTES_B);

            // Check deleting non-existent key
            redisQueue.del("blahblah");
            std::vector<uint8_t> actualDel2 = redisQueue.get("blahblah");
            REQUIRE(actualDel2.empty());
        }

        SECTION("Test set range") {
            std::string key = "setrange_test";

            std::string initialValue = "hello there world!";
            std::vector<uint8_t> bytesValue = util::stringToBytes(initialValue);
            redisQueue.set(key, bytesValue);

            REQUIRE(redisQueue.get(key) == bytesValue);

            std::string replacement = "hello";
            std::vector<uint8_t> replacementBytes = util::stringToBytes(replacement);
            redisQueue.setRange(key, 6, replacementBytes.data(), replacementBytes.size());

            std::string expected = "hello hello world!";
            std::vector<uint8_t> expectedBytes = util::stringToBytes(expected);
            REQUIRE(redisQueue.get(key) == expectedBytes);
        }

        SECTION("Test get range") {
            std::string key = "getrange_test";

            std::string initialValue = "get this string!";
            std::vector<uint8_t> bytesValue = util::stringToBytes(initialValue);
            redisQueue.set(key, bytesValue);

            REQUIRE(redisQueue.get(key) == bytesValue);

            // Note that redis getrange is inclusive on start/end indices
            uint8_t buffer[3];
            redisQueue.getRange(key, buffer, 3, 4, 6);
            std::vector<uint8_t> actualBytes(buffer, buffer + 3);

            std::string expected = "thi";
            std::vector<uint8_t> expectedBytes = util::stringToBytes(expected);
            REQUIRE(actualBytes == expectedBytes);
        }

        SECTION("Test get empty key") {
            std::string nonExistKey = "blahblah";
            std::vector<uint8_t> actual = redisQueue.get(nonExistKey);
            REQUIRE(actual.empty());
        }
    }

    TEST_CASE("Test setnxex", "[redis]") {
        Redis &redisState = Redis::getState();
        redisState.flushAll();

        std::string key = "setnxex_test";
        long valueA = 101;
        long valueB = 102;

        // Set one value and make sure it works
        long res = redisState.setnxex(key, valueA, 60);
        REQUIRE(res);
        REQUIRE(redisState.getLong(key) == valueA);

        // Try to set another and make sure it doesn't
        long res2 = redisState.setnxex(key, valueB, 60);
        REQUIRE(!res2);
        REQUIRE(redisState.getLong(key) == valueA);

        // Remove first value, try again and make sure it works
        redisState.del(key);
        long res3 = redisState.setnxex(key, valueB, 60);
        REQUIRE(res3);
        REQUIRE(redisState.getLong(key) == valueB);
    }

    void checkDelIfEq(Redis &redis) {
        redis.flushAll();

        std::string key = "delifeq_test";
        long valueA = 101;
        long valueB = 102;

        // Try with nothing set
        redis.delIfEq(key, valueA);
        REQUIRE(redis.get(key).empty());

        // Set a value and try deleting another
        redis.setnxex(key, valueA, 60);
        redis.delIfEq(key, valueB);
        REQUIRE(redis.getLong(key) == valueA);

        // Now delete the actual value
        redis.delIfEq(key, valueA);
        REQUIRE(redis.get(key).empty());
    }

    TEST_CASE("Test del if equal for state", "[redis]") {
        checkDelIfEq(Redis::getState());
    }

    TEST_CASE("Test del if equal for queue", "[redis]") {
        checkDelIfEq(Redis::getQueue());
    }

    void checkLock(Redis &redis) {
        redis.flushAll();

        std::string key = "lock_test";
        std::string lockKey = key + "_lock";

        // Set a value for the key to make sure it isn't affected
        std::vector<uint8_t> value = {0, 1, 2, 3};
        redis.set(key, value);

        // Check releasing when lock doesn't exist does nothing
        redis.releaseLock(key, 1234);

        // Check we can acquire the lock
        long lockId = redis.acquireLock(key, 10);
        REQUIRE(lockId > 0);
        REQUIRE(redis.getLong(lockKey) == lockId);
        REQUIRE(redis.get(key) == value);

        // Check someone else can't acquire the lock
        long lockId2 = redis.acquireLock(key, 10);
        REQUIRE(lockId2 == 0);
        REQUIRE(redis.getLong(lockKey) == lockId);
        REQUIRE(redis.get(key) == value);

        // Release with an invalid lock ID and check it's still locked
        redis.releaseLock(key, lockId + 1);
        long lockId3 = redis.acquireLock(key, 10);
        REQUIRE(lockId3 == 0);
        REQUIRE(redis.getLong(lockKey) == lockId);
        REQUIRE(redis.get(key) == value);

        // Release with valid ID and check we can re-acquire
        redis.releaseLock(key, lockId);
        long lockId4 = redis.acquireLock(key, 10);
        REQUIRE(lockId4 > 0);
        REQUIRE(lockId4 != lockId);
        REQUIRE(redis.getLong(lockKey) == lockId4);
        REQUIRE(redis.get(key) == value);
    }

    TEST_CASE("Test acquire/ release lock for state", "[redis]") {
        checkLock(Redis::getState());
    }

    TEST_CASE("Test acquire/ release lock for queue", "[redis]") {
        checkLock(Redis::getQueue());
    }

    void checkConditionalLock(Redis &redis) {
        redis.flushAll();

        std::string key = "lock_test";
        std::string lockKey = key + "_lock";

        // Check we can acquire the lock when the key doesn't exist
        long lockId = redis.acquireConditionalLock(key, 10);
        REQUIRE(lockId > 0);
        REQUIRE(redis.getLong(lockKey) == lockId);
        REQUIRE(redis.getLong(key) == 0);

        // Release it
        redis.releaseLock(key, lockId);

        // Set some value and check we can get the lock with the right value
        redis.setLong(key, 50);
        long lockId2 = redis.acquireConditionalLock(key, 50);
        REQUIRE(lockId2 > 0);
        REQUIRE(redis.getLong(lockKey) == lockId2);
        REQUIRE(redis.getLong(key) == 50);

        // Check someone else can't get it with the same value
        long lockId3 = redis.acquireConditionalLock(key, 50);
        REQUIRE(lockId3 == 0);
        REQUIRE(redis.getLong(lockKey) == lockId2);
        REQUIRE(redis.getLong(key) == 50);

        // Release it
        redis.releaseLock(key, lockId2);

        // Check we can't acquire it with the wrong value
        redis.setLong(key, 50);
        long lockId4 = redis.acquireConditionalLock(key, 20);
        REQUIRE(lockId4 == -1);
        REQUIRE(redis.getLong(lockKey) == 0);
        REQUIRE(redis.getLong(key) == 50);
    }

    TEST_CASE("Test acquire/ release conditional lock for state", "[redis]") {
        checkConditionalLock(Redis::getState());
    }

    TEST_CASE("Test acquire/ release conditional lock for queue", "[redis]") {
        checkConditionalLock(Redis::getQueue());
    }

    TEST_CASE("Test set operations with empty sets", "[redis]") {
        Redis &redis = Redis::getQueue();
        redis.flushAll();

        std::string setName = "test_empty_set";

        std::unordered_set<std::string> members = redis.smembers(setName);
        REQUIRE(members.empty());
    }

    TEST_CASE("Test set operations", "[redis]") {
        Redis &redisQueue = Redis::getQueue();
        redisQueue.flushAll();

        std::string setA = "set_a";
        std::string setB = "set_b";

        std::string valueA = "val_a";
        std::string valueB = "val_b";
        std::string valueC = "val_c";

        // Check cardinality is zero for both initially
        REQUIRE(redisQueue.scard(setA) == 0);
        REQUIRE(redisQueue.scard(setB) == 0);
        REQUIRE(!redisQueue.sismember(setA, valueA));
        REQUIRE(!redisQueue.sismember(setA, valueB));

        // Add something and check
        redisQueue.sadd(setA, valueA);
        REQUIRE(redisQueue.scard(setA) == 1);
        REQUIRE(redisQueue.sismember(setA, valueA));
        REQUIRE(!redisQueue.sismember(setA, valueB));
        REQUIRE(redisQueue.scard(setB) == 0);

        // Add more and check
        redisQueue.sadd(setA, valueB);
        redisQueue.sadd(setB, valueC);
        REQUIRE(redisQueue.scard(setA) == 2);
        REQUIRE(redisQueue.sismember(setA, valueA));
        REQUIRE(redisQueue.sismember(setA, valueB));
        REQUIRE(redisQueue.sismember(setB, valueC));
        REQUIRE(redisQueue.scard(setB) == 1);

        // Remove and check
        redisQueue.srem(setA, valueB);
        REQUIRE(redisQueue.scard(setA) == 1);
        REQUIRE(redisQueue.sismember(setA, valueA));
        REQUIRE(!redisQueue.sismember(setA, valueB));
        REQUIRE(redisQueue.sismember(setB, valueC));
        REQUIRE(redisQueue.scard(setB) == 1);
    }

    TEST_CASE("Test set random member", "[redis]") {
        Redis &redisQueue = Redis::getQueue();
        redisQueue.flushAll();

        std::string setName = "set_foo";
        REQUIRE(redisQueue.srandmember(setName).empty());

        std::string valueA = "val_a";
        std::string valueB = "val_b";
        redisQueue.sadd(setName, valueA);

        REQUIRE(redisQueue.srandmember(setName) == valueA);

        redisQueue.sadd(setName, valueB);
        const std::string actual = redisQueue.srandmember(setName);
        bool isExpected = (actual == valueA) || (actual == valueB);
        REQUIRE(isExpected);
    }

    TEST_CASE("Test set diff", "[redis]") {
        Redis &redisQueue = Redis::getQueue();
        redisQueue.flushAll();

        std::string setA = "set_a";
        std::string setB = "set_b";

        const std::unordered_set<std::string> actualA = redisQueue.sdiff(setA, setB);
        REQUIRE(actualA.empty());

        std::string valueA = "val_a";
        std::string valueB = "val_b";
        std::string valueC = "val_c";
        std::string valueD = "val_d";

        redisQueue.sadd(setA, valueA);
        redisQueue.sadd(setA, valueB);
        redisQueue.sadd(setA, valueC);

        redisQueue.sadd(setB, valueC);
        redisQueue.sadd(setB, valueD);

        std::unordered_set<std::string> actualB = redisQueue.sdiff(setA, setB);
        std::unordered_set<std::string> expected = {valueA, valueB};

        REQUIRE(actualB == expected);
    }

    TEST_CASE("Test set intersection", "[redis]") {
        Redis &redisQueue = Redis::getQueue();
        redisQueue.flushAll();

        std::string setA = "set_a";
        std::string setB = "set_b";

        std::unordered_set<std::string> actualA = redisQueue.sinter(setA, setB);
        REQUIRE(actualA.empty());

        std::string valueA = "val_a";
        std::string valueB = "val_b";
        std::string valueC = "val_c";
        std::string valueD = "val_d";
        std::string valueE = "val_e";

        redisQueue.sadd(setA, valueA);
        redisQueue.sadd(setA, valueB);
        redisQueue.sadd(setA, valueC);

        redisQueue.sadd(setB, valueA);
        redisQueue.sadd(setB, valueC);
        redisQueue.sadd(setB, valueD);
        redisQueue.sadd(setB, valueE);

        std::unordered_set<std::string> actualB = redisQueue.sinter(setA, setB);
        std::unordered_set<std::string> expected = {valueA, valueC};

        REQUIRE(actualB == expected);
    }
}