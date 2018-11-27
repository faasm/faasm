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

    void checkBindMessageDispatched(Redis &cli, const message::Message &expected, int expectedTarget) {
        const message::Message actual = cli.nextMessage(infra::PREWARM_QUEUE);
        REQUIRE(actual.user() == expected.user());
        REQUIRE(actual.function() == expected.function());
        REQUIRE(actual.type() == message::Message_MessageType_BIND);
        REQUIRE(actual.target() == expectedTarget);
    }

    TEST_CASE("Test calling function with no workers sends bind message", "[redis]") {
        Redis cli;
        cli.flushAll();

        message::Message call;
        call.set_function("my func");
        call.set_user("some user");

        cli.callFunction(call);

        checkBindMessageDispatched(cli, call, 1);
    }

    TEST_CASE("Test calling function with existing workers does not send bind message", "[redis]") {
        Redis cli;
        cli.flushAll();

        message::Message call;
        call.set_function("my func");
        call.set_user("some user");

        std::string queueName = getFunctionQueueName(call);
        std::string setName = getFunctionSetName(call);

        // Add 2 fake workers
        cli.sadd(setName, "worker 1");
        cli.sadd(setName, "worker 2");

        // Call the function
        cli.callFunction(call);

        // Check function call has been added, but no bind messages
        REQUIRE(cli.listLength(PREWARM_QUEUE) == 0);
        REQUIRE(cli.listLength(queueName) == 1);
    }

    TEST_CASE("Test calling function which breaches queue ratio sends bind message", "[redis]") {
        Redis cli;
        cli.flushAll();

        message::Message call;
        call.set_function("my func");
        call.set_user("some user");

        std::string queueName = getFunctionQueueName(call);
        std::string setName = getFunctionSetName(call);

        // Add 2 fake workers
        cli.sadd(setName, "worker 1");
        cli.sadd(setName, "worker 2");

        // Saturate up to the number of max queued calls
        int nCalls = infra::MAX_QUEUE_RATIO * 2;
        for(int i =0; i < nCalls; i++) {
            cli.callFunction(call);
        }

        // Check no bind messages
        REQUIRE(cli.listLength(PREWARM_QUEUE) == 0);

        // Check all calls have been added to queue
        REQUIRE(cli.listLength(queueName) == nCalls);

        // Dispatch another and check that a bind message is sent with correct target
        cli.callFunction(call);
        REQUIRE(cli.listLength(PREWARM_QUEUE) == 1);
        REQUIRE(cli.listLength(queueName) == nCalls + 1);

        checkBindMessageDispatched(cli, call, 3);
    }
}