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

    TEST_CASE("Test adding to and removing from set", "[redis]") {
        Redis cli;
        cli.flushAll();

        std::string setName = "test_set";

        // Check set empty initially
        REQUIRE(cli.spop(setName).empty());

        // Add a few values
        std::string valA = "valA";
        std::string valB = "valB";
        std::string valC = "valC";
        cli.sadd(setName, valA);
        cli.sadd(setName, valB);
        cli.sadd(setName, valC);

        // Explicitly remove one
        cli.srem(setName, valB);

        // Get two randoms
        std::string actualA = cli.spop(setName);
        std::string actualC = cli.spop(setName);

        std::set<std::string> actuals = {actualA, actualC};

        // Check both are present in the actuals
        REQUIRE(actuals.find(valA) != actuals.end());
        REQUIRE(actuals.find(valC) != actuals.end());

        // Check set is now empty
        REQUIRE(cli.spop(setName).empty());
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

        cli.callFunction(call);

        // Check one function call on the queue
        std::string queueName = infra::getFunctionQueueName(call);
        REQUIRE(cli.listLength(queueName) == 1);

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

    message::Message callFunction(Redis *cli) {
        cli->flushAll();

        // Write some function result
        message::Message call;
        call.set_function("my func");
        call.set_user("some user");
        call.set_resultkey("function 123");

        cli->setFunctionResult(call, true);

        // Check result has been written to the right key
        REQUIRE(cli->listLength("function 123") == 1);

        // Check that some expiry has been set
        long ttl = cli->getTtl(call.resultkey());
        REQUIRE(ttl > 10);

        return call;
    }

    TEST_CASE("Test redis reading and writing function results", "[redis]") {
        Redis cli;
        message::Message call = callFunction(&cli);

        // Check retrieval method gets the same call out again
        message::Message actualCall2 = cli.getFunctionResult(call);

        REQUIRE("my func" == actualCall2.function());
        REQUIRE("some user" == actualCall2.user());
        REQUIRE("function 123" == actualCall2.resultkey());
        REQUIRE(actualCall2.success());
    }

}