#include <catch/catch.hpp>
#include <infra/infra.h>
#include <util/util.h>
#include "utils.h"


using namespace infra;

namespace tests {

    namespace vars {
        static const std::string QUEUE_NAME = "my queue";

        static const std::string VALUE_A = "val a";
        static const std::string VALUE_B = "val b";
        static const std::string VALUE_C = "val c";

        static const std::vector<uint8_t> BYTES_A = util::stringToBytes(VALUE_A);
        static const std::vector<uint8_t> BYTES_B = util::stringToBytes(VALUE_B);
        static const std::vector<uint8_t> BYTES_C = util::stringToBytes(VALUE_C);
    }

    static void doEnqueue() {
        redisQueue.flushAll();

        // Enqueue some values
        std::vector<uint8_t> bytesA = util::stringToBytes(vars::VALUE_A);
        redisQueue.enqueue(vars::QUEUE_NAME, vars::BYTES_A);
        redisQueue.enqueue(vars::QUEUE_NAME, vars::BYTES_B);
        redisQueue.enqueue(vars::QUEUE_NAME, vars::BYTES_C);

        // Check expected length
        REQUIRE(3 == redisQueue.listLength(vars::QUEUE_NAME));
    }

    TEST_CASE("Test incr/ decr", "[redis]") {
        redisQueue.flushAll();

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

    TEST_CASE("Test enqueue/ dequeue", "[redis]") {
        // Enqueue
        doEnqueue();

        // Blocking dequeue
        std::vector<uint8_t> actual = redisQueue.dequeue(vars::QUEUE_NAME);

        REQUIRE(vars::BYTES_A == actual);

        // Dequeue again
        std::vector<uint8_t> actual2 = redisQueue.dequeue(vars::QUEUE_NAME);

        REQUIRE(vars::BYTES_B == actual2);
    }

    TEST_CASE("Test set/ get/ del", "[redis]") {
        redisQueue.flushAll();

        std::string keyA = "key a";
        std::string keyB = "key b";

        redisQueue.set(keyA, vars::BYTES_A);
        redisQueue.set(keyB, vars::BYTES_B);

        // Test multiple gets on the same key
        std::vector<uint8_t> actualA1 = redisQueue.get(keyA);
        std::vector<uint8_t> actualA2 = redisQueue.get(keyA);
        std::vector<uint8_t> actualB = redisQueue.get(keyB);

        REQUIRE(vars::BYTES_A == actualA1);
        REQUIRE(vars::BYTES_A == actualA2);

        REQUIRE(vars::BYTES_B == actualB);

        // Check deleting
        redisQueue.del(keyA);
        std::vector<uint8_t> actualDel = redisQueue.get(keyA);
        REQUIRE(actualDel.empty());

        // Check other still there
        REQUIRE(redisQueue.get(keyB) == vars::BYTES_B);

        // Check deleting non-existent key
        redisQueue.del("blahblah");
        std::vector<uint8_t> actualDel2 = redisQueue.get("blahblah");
        REQUIRE(actualDel2.empty());
    }

    TEST_CASE("Test set range", "[redis]") {

        redisQueue.flushAll();

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

    TEST_CASE("Test get range", "[redis]") {

        redisQueue.flushAll();

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

    TEST_CASE("Test get empty key", "[redis]") {
        redisQueue.flushAll();

        std::string nonExistKey = "blahblah";
        std::vector<uint8_t> actual = redisQueue.get(nonExistKey);
        REQUIRE(actual.empty());
    }

    void checkCallRoundTrip(bool isAsync, bool isSuccess) {
        redisQueue.flushAll();

        // Request function
        message::Message call;
        std::string funcName = "my func";
        std::string userName = "some user";
        std::string inputData = "blahblah";

        call.set_function(funcName);
        call.set_user(userName);
        call.set_inputdata(inputData);
        call.set_isasync(isAsync);
        call.set_success(isSuccess);

        // Check resultkey not set initially
        REQUIRE(!call.has_resultkey());

        std::string queueName = infra::Scheduler::callFunction(call);

        // Get the next call
        message::Message actual = redisQueue.nextMessage(queueName);

        REQUIRE(funcName == actual.function());
        REQUIRE(userName == actual.user());
        REQUIRE(inputData == actual.inputdata());
        REQUIRE(isAsync == actual.isasync());
        REQUIRE(isSuccess == actual.success());

        // Check result key has now been set
        REQUIRE(actual.has_resultkey());
    }

    TEST_CASE("Test redis round trip sync fail", "[redis]") {
        checkCallRoundTrip(false, false);
    }

    TEST_CASE("Test redis round trip sync success", "[redis]") {
        checkCallRoundTrip(false, true);
    }

    TEST_CASE("Test redis round trip async fail", "[redis]") {
        checkCallRoundTrip(true, false);
    }

    TEST_CASE("Test redis round trip async success", "[redis]") {
        checkCallRoundTrip(true, true);
    }

    TEST_CASE("Test redis reading and writing function results", "[redis]") {
        redisQueue.flushAll();

        // Write some function result
        message::Message call;
        call.set_function("my func");
        call.set_user("some user");
        call.set_resultkey("function 123");

        redisQueue.setFunctionResult(call, true);

        // Check result has been written to the right key
        REQUIRE(redisQueue.listLength("function 123") == 1);

        // Check that some expiry has been set
        long ttl = redisQueue.getTtl(call.resultkey());
        REQUIRE(ttl > 10);

        // Check retrieval method gets the same call out again
        message::Message actualCall2 = redisQueue.getFunctionResult(call);

        REQUIRE("my func" == actualCall2.function());
        REQUIRE("some user" == actualCall2.user());
        REQUIRE("function 123" == actualCall2.resultkey());
        REQUIRE(actualCall2.success());
    }

    void _validateAddWorker(int nWorkers, int queueLengh, int maxRatio, int maxWorkers, bool expectIncrement) {
        redisQueue.flushAll();

        std::string counter = "dummyCount";
        std::string queueName = "dummyQueue";

        // Set up number of workers
        for (int w = 0; w < nWorkers; w++) {
            redisQueue.incr(counter);
        }

        // Set up queue
        for (int q = 0; q < queueLengh; q++) {
            redisQueue.enqueue(queueName, {0, 1});
        }

        // Call addWorker
        bool shouldAddWorker = redisQueue.addWorker(counter, queueName, maxRatio, maxWorkers);

        // Check queue length hasn't changed
        REQUIRE(redisQueue.listLength(queueName) == queueLengh);

        if (expectIncrement) {
            // Check result indicates increment
            REQUIRE(shouldAddWorker);

            // Check worker count has been incremented
            REQUIRE(redisQueue.getCounter(counter) == nWorkers + 1);
        } else {
            // Check no change
            REQUIRE(!shouldAddWorker);
            REQUIRE(redisQueue.getCounter(counter) == nWorkers);
        }
    }


    TEST_CASE("Test add worker with zero counter causes addition", "[redis]") {
        int nWorkers = 0;
        int queueLength = 1;
        int maxRatio = 5;
        int maxWorkers = 10;
        bool expectIncrement = true;

        _validateAddWorker(nWorkers, queueLength, maxRatio, maxWorkers, expectIncrement);
    }

    TEST_CASE("Test add worker with queue but below threshold causes no addition", "[redis]") {
        int nWorkers = 1;
        int queueLength = 1;
        int maxRatio = 5;
        int maxWorkers = 10;
        bool expectIncrement = false;

        _validateAddWorker(nWorkers, queueLength, maxRatio, maxWorkers, expectIncrement);
    }

    TEST_CASE("Test add worker above threshold causes addition", "[redis]") {
        int nWorkers = 1;
        int queueLength = 6;
        int maxRatio = 5;
        int maxWorkers = 10;
        bool expectIncrement = true;

        _validateAddWorker(nWorkers, queueLength, maxRatio, maxWorkers, expectIncrement);
    }

    TEST_CASE("Test add worker above threshold but above worker limit causes no addition", "[redis]") {
        int nWorkers = 6;
        int queueLength = 100;
        int maxRatio = 5;
        int maxWorkers = 5;
        bool expectIncrement = false;

        _validateAddWorker(nWorkers, queueLength, maxRatio, maxWorkers, expectIncrement);
    }

    void _validateIncrIfBelowTarget(int currentValue, int target, bool expectIncr) {
        redisQueue.flushAll();

        std::string key = "incr_test";

        for (int i = 0; i < currentValue; i++) {
            redisQueue.incr(key);
        }

        bool res = redisQueue.incrIfBelowTarget(key, target);

        if (expectIncr) {
            REQUIRE(res);
            REQUIRE(redisQueue.getCounter(key) == currentValue + 1);
        } else {
            REQUIRE(!res);
            REQUIRE(redisQueue.getCounter(key) == currentValue);
        }
    }

    TEST_CASE("Test incr below target increases if uninitialised", "[redis]") {
        _validateIncrIfBelowTarget(0, 1, true);
    }

    TEST_CASE("Test incr below target increases if below target", "[redis]") {
        _validateIncrIfBelowTarget(10, 12, true);
    }

    TEST_CASE("Test incr below target doesn't increase if equal", "[redis]") {
        _validateIncrIfBelowTarget(10, 10, false);
    }

    TEST_CASE("Test incr below target doesn't increase if above", "[redis]") {
        _validateIncrIfBelowTarget(15, 10, false);
    }

    TEST_CASE("Test setnxex", "[redis]") {
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

    TEST_CASE("Test del if equal", "[redis]") {
        redisState.flushAll();

        std::string key = "delifeq_test";
        long valueA = 101;
        long valueB = 102;

        // Try with nothing set
        redisState.delIfEq(key, valueA);
        REQUIRE(redisState.get(key).empty());

        // Set a value and try deleting another
        redisState.setnxex(key, valueA, 60);
        redisState.delIfEq(key, valueB);
        REQUIRE(redisState.getLong(key) == valueA);

        // Now delete the actual value
        redisState.delIfEq(key, valueA);
        REQUIRE(redisState.get(key).empty());
    }

    TEST_CASE("Test acquire/ release lock", "[redis]") {
        redisState.flushAll();

        std::string key = "lock_test";
        std::string lockKey = key + "_lock";

        // Set a value for the key to make sure it isn't affected
        std::vector<uint8_t> value = {0, 1, 2, 3};
        redisState.set(key, value);

        // Check we can acquire the lock
        long lockId = redisState.acquireLock(key, 10);
        REQUIRE(lockId > 0);
        REQUIRE(redisState.getLong(lockKey) == lockId);
        REQUIRE(redisState.get(key) == value);

        // Check someone else can't acquire the lock
        long lockId2 = redisState.acquireLock(key, 10);
        REQUIRE(lockId2 == -1);
        REQUIRE(redisState.getLong(lockKey) == lockId);
        REQUIRE(redisState.get(key) == value);

        // Release with an invalid lock ID and check it's still locked
        redisState.releaseLock(key, lockId + 1);
        long lockId3 = redisState.acquireLock(key, 10);
        REQUIRE(lockId3 == -1);
        REQUIRE(redisState.getLong(lockKey) == lockId);
        REQUIRE(redisState.get(key) == value);

        // Release with valid ID and check we can re-acquire
        redisState.releaseLock(key, lockId);
        long lockId4 = redisState.acquireLock(key, 10);
        REQUIRE(lockId4 > 0);
        REQUIRE(lockId4 != lockId);
        REQUIRE(redisState.getLong(lockKey) == lockId4);
        REQUIRE(redisState.get(key) == value);
    }
}