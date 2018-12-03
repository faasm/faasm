#include <catch/catch.hpp>
#include <infra/infra.h>
#include <util/util.h>

using namespace infra;

namespace tests {

    namespace vars {
        const std::string QUEUE_NAME = "my queue";

        const std::string VALUE_A = "val a";
        const std::string VALUE_B = "val b";
        const std::string VALUE_C = "val c";

        const std::vector<uint8_t> BYTES_A = util::stringToBytes(VALUE_A);
        const std::vector<uint8_t> BYTES_B = util::stringToBytes(VALUE_B);
        const std::vector<uint8_t> BYTES_C = util::stringToBytes(VALUE_C);
    }

    void doEnqueue(Redis *cli) {
        cli->flushAll();

        // Enqueue some values
        std::vector<uint8_t> bytesA = util::stringToBytes(vars::VALUE_A);
        cli->enqueue(vars::QUEUE_NAME, vars::BYTES_A);
        cli->enqueue(vars::QUEUE_NAME, vars::BYTES_B);
        cli->enqueue(vars::QUEUE_NAME, vars::BYTES_C);

        // Check expected length
        REQUIRE(3 == cli->listLength(vars::QUEUE_NAME));
    }

    TEST_CASE("Test incr/ decr", "[redis]") {
        Redis cli;
        cli.flushAll();

        std::string key = "test_counter";

        REQUIRE(cli.getCounter(key) == 0);

        REQUIRE(cli.incr(key) == 1);
        REQUIRE(cli.getCounter(key) == 1);

        REQUIRE(cli.incr(key) == 2);
        REQUIRE(cli.incr(key) == 3);
        REQUIRE(cli.incr(key) == 4);
        REQUIRE(cli.getCounter(key) == 4);

        REQUIRE(cli.decr(key) == 3);
        REQUIRE(cli.decr(key) == 2);
        REQUIRE(cli.getCounter(key) == 2);
    }

    TEST_CASE("Test enqueue/ dequeue", "[redis]") {
        Redis cli;

        // Enqueue
        doEnqueue(&cli);

        // Blocking dequeue
        std::vector<uint8_t> actual = cli.dequeue(vars::QUEUE_NAME);

        REQUIRE(vars::BYTES_A == actual);

        // Dequeue again
        std::vector<uint8_t> actual2 = cli.dequeue(vars::QUEUE_NAME);

        REQUIRE(vars::BYTES_B == actual2);
    }

    TEST_CASE("Test set/ get/ del", "[redis]") {
        Redis cli;
        cli.flushAll();

        std::string keyA = "key a";
        std::string keyB = "key b";

        cli.set(keyA, vars::BYTES_A);
        cli.set(keyB, vars::BYTES_B);

        // Test multiple gets on the same key
        std::vector<uint8_t> actualA1 = cli.get(keyA);
        std::vector<uint8_t> actualA2 = cli.get(keyA);
        std::vector<uint8_t> actualB = cli.get(keyB);

        REQUIRE(vars::BYTES_A == actualA1);
        REQUIRE(vars::BYTES_A == actualA2);

        REQUIRE(vars::BYTES_B == actualB);

        // Check deleting
        cli.del(keyA);
        std::vector<uint8_t> actualDel = cli.get(keyA);
        REQUIRE(actualDel.empty());

        // Check other still there
        REQUIRE(cli.get(keyB) == vars::BYTES_B);

        // Check deleting non-existent key
        cli.del("blahblah");
        std::vector<uint8_t> actualDel2 = cli.get("blahblah");
        REQUIRE(actualDel2.empty());
    }

    TEST_CASE("Test set range", "[redis]") {
        Redis cli;
        cli.flushAll();
        
        std::string key = "setrange_test";
        
        std::string initialValue = "hello there world!";
        std::vector<uint8_t> bytesValue = util::stringToBytes(initialValue);
        cli.set(key, bytesValue);
        
        REQUIRE(cli.get(key) == bytesValue);

        std::string replacement = "hello";
        std::vector<uint8_t> replacementBytes = util::stringToBytes(replacement);
        cli.setRange(key, 6, replacementBytes);

        std::string expected = "hello hello world!";
        std::vector<uint8_t> expectedBytes = util::stringToBytes(expected);
        REQUIRE(cli.get(key) == expectedBytes);
    }

    TEST_CASE("Test get range", "[redis]") {
        Redis cli;
        cli.flushAll();

        std::string key = "getrange_test";

        std::string initialValue = "get this string!";
        std::vector<uint8_t> bytesValue = util::stringToBytes(initialValue);
        cli.set(key, bytesValue);

        REQUIRE(cli.get(key) == bytesValue);

        std::vector<uint8_t> actualBytes = cli.getRange(key, 4, 7);

        std::string expected = "this";
        std::vector<uint8_t> expectedBytes = util::stringToBytes(expected);
        REQUIRE(actualBytes == expectedBytes);
    }

    TEST_CASE("Test get empty key", "[redis]") {
        Redis cli;
        cli.flushAll();

        std::string nonExistKey = "blahblah";
        std::vector<uint8_t> actual = cli.get(nonExistKey);
        REQUIRE(actual.empty());
    }

    void checkCallRoundTrip(bool isAsync, bool isSuccess) {
        Redis cli;
        cli.flushAll();

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
        message::Message actual = cli.nextMessage(queueName);

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
        Redis cli;
        cli.flushAll();

        // Write some function result
        message::Message call;
        call.set_function("my func");
        call.set_user("some user");
        call.set_resultkey("function 123");

        cli.setFunctionResult(call, true);

        // Check result has been written to the right key
        REQUIRE(cli.listLength("function 123") == 1);

        // Check that some expiry has been set
        long ttl = cli.getTtl(call.resultkey());
        REQUIRE(ttl > 10);

        // Check retrieval method gets the same call out again
        message::Message actualCall2 = cli.getFunctionResult(call);

        REQUIRE("my func" == actualCall2.function());
        REQUIRE("some user" == actualCall2.user());
        REQUIRE("function 123" == actualCall2.resultkey());
        REQUIRE(actualCall2.success());
    }

    void _validateAddWorker(int nWorkers, int queueLengh, int maxRatio, int maxWorkers, bool expectIncrement) {
        Redis cli;
        cli.flushAll();

        std::string counter = "dummyCount";
        std::string queueName = "dummyQueue";

        // Set up number of workers
        for(int w=0; w < nWorkers; w++) {
            cli.incr(counter);
        }

        // Set up queue
        for(int q=0; q < queueLengh; q++) {
            cli.enqueue(queueName, {0, 1});
        }

        // Call addWorker
        long actual = cli.addWorker(counter, queueName, maxRatio, maxWorkers);

        // Check queue length hasn't changed
        REQUIRE(cli.listLength(queueName) == queueLengh);

        if(expectIncrement) {
            // Check result indicates increment
            REQUIRE(actual == 1);

            // Check worker count has been incremented
            REQUIRE(cli.getCounter(counter) == nWorkers + 1);
        }
        else {
            // Check no change
            REQUIRE(actual == 0);
            REQUIRE(cli.getCounter(counter) == nWorkers);
        }
    }


    TEST_CASE("Test add worker with zero counter causes addition", "[redis]") {
        int nWorkers = 0;
        int queueLength = 1;
        int maxRatio = 5;
        int maxWorkers =10;
        bool expectIncrement = true;

        _validateAddWorker(nWorkers, queueLength, maxRatio, maxWorkers, expectIncrement);
    }

    TEST_CASE("Test add worker with queue but below threshold causes no addition", "[redis]") {
        int nWorkers = 1;
        int queueLength = 1;
        int maxRatio = 5;
        int maxWorkers =10;
        bool expectIncrement = false;

        _validateAddWorker(nWorkers, queueLength, maxRatio, maxWorkers, expectIncrement);
    }

    TEST_CASE("Test add worker above threshold causes addition", "[redis]") {
        int nWorkers = 1;
        int queueLength = 6;
        int maxRatio = 5;
        int maxWorkers =10;
        bool expectIncrement = true;

        _validateAddWorker(nWorkers, queueLength, maxRatio, maxWorkers, expectIncrement);
    }

    TEST_CASE("Test add worker above threshold but above worker limit causes no addition", "[redis]") {
        int nWorkers = 6;
        int queueLength = 100;
        int maxRatio = 5;
        int maxWorkers =5;
        bool expectIncrement = false;

        _validateAddWorker(nWorkers, queueLength, maxRatio, maxWorkers, expectIncrement);
    }

}