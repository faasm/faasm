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
    
    TEST_CASE("Test adding to unassigned set", "[redis]") {
        Redis cli;
        cli.flushAll();
        
        std::string queueNameA = "flahblah";
        std::string queueNameB = "clahdlah";
        std::set<std::string> expected = {queueNameA, queueNameB};

        cli.addToUnassignedSet(queueNameA);

        REQUIRE(cli.scard(UNASSIGNED_SET) == 1);
        REQUIRE(cli.sismember(UNASSIGNED_SET, queueNameA));
        REQUIRE(!cli.sismember(UNASSIGNED_SET, queueNameB));

        cli.addToUnassignedSet(queueNameB);

        REQUIRE(cli.scard(UNASSIGNED_SET) == 2);
        REQUIRE(cli.sismember(UNASSIGNED_SET, queueNameA));
        REQUIRE(cli.sismember(UNASSIGNED_SET, queueNameB));

        cli.removeFromUnassignedSet(queueNameA);
        REQUIRE(!cli.sismember(UNASSIGNED_SET, queueNameA));
        REQUIRE(cli.sismember(UNASSIGNED_SET, queueNameB));
        REQUIRE(cli.scard(UNASSIGNED_SET) == 1);
    }

    TEST_CASE("Test adding to/ removing from function set", "[redis]") {
        Redis cli;
        cli.flushAll();

        std::string queueNameA = "flahblah";
        std::string queueNameB = "clahdlah";

        message::FunctionCall callA;
        callA.set_user("user_a");
        callA.set_function("func_a");

        std::string funcSetA = getFunctionSetName(callA);

        // Check empty to start with
        REQUIRE(cli.scard(funcSetA) == 0);

        // Add to sets and check
        cli.addToFunctionSet(callA, queueNameA);
        REQUIRE(cli.sismember(funcSetA, queueNameA));
        REQUIRE(!cli.sismember(funcSetA, queueNameB));
        REQUIRE(cli.scard(funcSetA) == 1);

        cli.addToFunctionSet(callA, queueNameB);
        REQUIRE(cli.sismember(funcSetA, queueNameA));
        REQUIRE(cli.sismember(funcSetA, queueNameB));
        REQUIRE(cli.scard(funcSetA) == 2);

        // Remove and check
        cli.removeFromFunctionSet(callA, queueNameA);
        REQUIRE(!cli.sismember(funcSetA, queueNameA));
        REQUIRE(cli.sismember(funcSetA, queueNameB));
        REQUIRE(cli.scard(funcSetA) == 1);
    }

    TEST_CASE("Test adding and removing from set", "[redis]") {
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

    TEST_CASE("Test getting queue for function", "[redis]") {
        Redis cli;
        cli.flushAll();

        std::string queueNameA = "aaa";
        std::string queueNameB = "bbb";
        std::string queueNameC = "ccc";
        std::string queueNameD = "ddd";

        message::FunctionCall callA;
        callA.set_user("user_a");
        callA.set_function("func_a");

        message::FunctionCall callB;
        callB.set_user("user_b");
        callB.set_function("func_b");

        // Add a the rest to assigned
        cli.addToUnassignedSet(queueNameA);
        cli.addToUnassignedSet(queueNameB);
        cli.addToUnassignedSet(queueNameC);
        std::set<std::string> unassignedQueues = {queueNameA, queueNameB, queueNameC};

        // Add one to the function set for one of the functions
        cli.addToFunctionSet(callA, queueNameD);
        
        // Get queue for the function with no assigned queue and check it's one
        // from the unassigned set
        const std::string actualUnassigned = cli.getQueueForFunc(callB);
        bool isFound = unassignedQueues.find(actualUnassigned) != unassignedQueues.end();
        REQUIRE(isFound);
        unassignedQueues.erase(actualUnassigned);

        // Get queue for function with an assigned queue and check it's the expected
        const std::string actualAssigned = cli.getQueueForFunc(callA);
        REQUIRE(actualAssigned == queueNameD);

        // Get another for this function, and check it draws from unassigned
        const std::string actualAssigned2 = cli.getQueueForFunc(callA);
        bool isFound2 = unassignedQueues.find(actualAssigned2) != unassignedQueues.end();
        REQUIRE(isFound2);
        unassignedQueues.erase(actualAssigned2);

        // Get another for the unassigned function and check it's the last unassigned value
        const std::string actualUnassigned2 = cli.getQueueForFunc(callB);
        REQUIRE(actualUnassigned2 == *unassignedQueues.begin());

        // Now try getting another for the assigned funciton and check we get an exception
        REQUIRE_THROWS(cli.getQueueForFunc(callA));
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
        message::FunctionCall call;
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

        // Make sure there's a worker available
        const std::string workerQueue = "dummyQ";
        cli.addToUnassignedSet(workerQueue);

        cli.callFunction(call);

        // Check one function call on there
        REQUIRE(cli.listLength(workerQueue) == 1);

        // Get the next call
        message::FunctionCall actual = cli.nextFunctionCall(workerQueue);

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

    message::FunctionCall callFunction(Redis *cli) {
        cli->flushAll();

        // Write some function result
        message::FunctionCall call;
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
        message::FunctionCall call = callFunction(&cli);

        // Check retrieval method gets the same call out again
        message::FunctionCall actualCall2 = cli.getFunctionResult(call);

        REQUIRE("my func" == actualCall2.function());
        REQUIRE("some user" == actualCall2.user());
        REQUIRE("function 123" == actualCall2.resultkey());
        REQUIRE(actualCall2.success());
    }

}