#include <catch/catch.hpp>
#include <infra/infra.h>
#include <util/util.h>
#include "utils.h"

using namespace infra;

namespace tests {
    TEST_CASE("Test getting timeout", "[scheduler]") {
        util::SystemConfig &conf = util::getSystemConfig();

        REQUIRE(Scheduler::getWorkerTimeout("blah") == conf.boundTimeout);

        std::string prewarmQueue = Scheduler::getHostPrewarmQueue("foo");
        REQUIRE(Scheduler::getWorkerTimeout(prewarmQueue) == conf.unboundTimeout);
    }

    TEST_CASE("Test worker finished bound", "[scheduler]") {
        redisQueue.flushAll();

        message::Message call;
        call.set_user("userA");
        call.set_function("funcA");

        // Call the function to simulate the worker getting added
        Scheduler::callFunction(call);
        REQUIRE(Scheduler::getFunctionCount(call) == 1);

        // Notify that a worker has finished
        Scheduler::workerFinished(Scheduler::getFunctionQueueName(call));

        // Check count decremented
        REQUIRE(Scheduler::getFunctionCount(call) == 0);
    }

    TEST_CASE("Test calling function with no workers sends bind message", "[scheduler]") {
        redisQueue.flushAll();

        message::Message call;
        call.set_function("my func");
        call.set_user("some user");

        REQUIRE(Scheduler::getFunctionCount(call) == 0);

        // Add a worker host to the global set
        redisQueue.sadd(GLOBAL_WORKER_SET, "foo");
        std::string expectedPrewarmQueue = Scheduler::getHostPrewarmQueue("foo");

        // Call the function
        Scheduler::callFunction(call);

        // Check function count has increased and bind message sent
        REQUIRE(Scheduler::getFunctionCount(call) == 1);

        REQUIRE(redisQueue.listLength(expectedPrewarmQueue) == 1);
    }

    TEST_CASE("Test sending bind messages", "[scheduler]") {
        redisQueue.flushAll();

        // Set up calls
        message::Message callA;
        callA.set_user("userA");
        callA.set_function("funcA");

        message::Message callB;
        callB.set_user("userB");
        callB.set_function("funcB");

        // Add a worker host to the global set
        redisQueue.sadd(GLOBAL_WORKER_SET, "foo");
        std::string expectedPrewarmQueue = Scheduler::getHostPrewarmQueue("foo");

        // Call each function
        Scheduler::callFunction(callA);
        Scheduler::callFunction(callB);

        // Check that counters are updated (should be done before bind is dispatched)
        REQUIRE(Scheduler::getFunctionCount(callA) == 1);
        REQUIRE(Scheduler::getFunctionCount(callB) == 1);

        // Check that bind messages have been sent
        const message::Message bindA = redisQueue.nextMessage(expectedPrewarmQueue);
        const message::Message bindB = redisQueue.nextMessage(expectedPrewarmQueue);

        REQUIRE(bindA.user() == callA.user());
        REQUIRE(bindA.function() == callA.function());
        REQUIRE(bindA.type() == message::Message_MessageType_BIND);

        REQUIRE(bindB.user() == callB.user());
        REQUIRE(bindB.function() == callB.function());
        REQUIRE(bindB.type() == message::Message_MessageType_BIND);
    }

    TEST_CASE("Test calling function with existing workers does not send bind message", "[scheduler]") {
        redisQueue.flushAll();

        message::Message call;
        call.set_function("my func");
        call.set_user("some user");

        // Add a worker host to the global set
        redisQueue.sadd(GLOBAL_WORKER_SET, "foo");
        std::string expectedPrewarmQueue = Scheduler::getHostPrewarmQueue("foo");

        std::string queueName = Scheduler::getFunctionQueueName(call);

        // Fake calling the function first, should cause a bind message and set up the function queue
        Scheduler::callFunction(call);
        REQUIRE(redisQueue.listLength(expectedPrewarmQueue) == 1);
        REQUIRE(redisQueue.listLength(queueName) == 1);

        // Call the function again
        Scheduler::callFunction(call);

        // Check function call has been added, but no new bind messages
        REQUIRE(redisQueue.listLength(expectedPrewarmQueue) == 1);
        REQUIRE(redisQueue.listLength(queueName) == 2);
    }

    TEST_CASE("Test calling function which breaches queue ratio sends bind message", "[scheduler]") {
        redisQueue.flushAll();

        message::Message call;
        call.set_function("my func");
        call.set_user("some user");

        std::string queueName = Scheduler::getFunctionQueueName(call);

        // Add a worker host to the global set
        redisQueue.sadd(GLOBAL_WORKER_SET, "foo");
        std::string expectedPrewarmQueue = Scheduler::getHostPrewarmQueue("foo");

        // Saturate up to the number of max queued calls
        util::SystemConfig &conf = util::getSystemConfig();
        int nCalls = conf.maxQueueRatio;
        for (int i = 0; i < nCalls; i++) {
            Scheduler::callFunction(call);

            // Check only one bind message is ever sent
            REQUIRE(redisQueue.listLength(expectedPrewarmQueue) == 1);

            // Check call queued
            REQUIRE(redisQueue.listLength(queueName) == i + 1);
        }

        // Dispatch another and check that a bind message is sent
        Scheduler::callFunction(call);
        REQUIRE(redisQueue.listLength(expectedPrewarmQueue) == 2);
        REQUIRE(redisQueue.listLength(queueName) == nCalls + 1);
    }

    TEST_CASE("Test function which breaches queue ratio but has no capacity does not scale up", "[scheduler]") {
        redisQueue.flushAll();

        message::Message call;
        call.set_user("userA");
        call.set_function("funcA");

        std::string queueName = Scheduler::getFunctionQueueName(call);

        // Add a worker host to the global set
        redisQueue.sadd(GLOBAL_WORKER_SET, "foo");
        std::string expectedPrewarmQueue = Scheduler::getHostPrewarmQueue("foo");

        // Make calls up to the limit
        util::SystemConfig &conf = util::getSystemConfig();
        int nCalls = conf.maxWorkersPerFunction * conf.maxQueueRatio;
        for (int i = 0; i < nCalls; i++) {
            Scheduler::callFunction(call);
        }

        // Check number of bind messages
        REQUIRE(redisQueue.listLength(expectedPrewarmQueue) == conf.maxWorkersPerFunction);

        // Check count assigned to function
        REQUIRE(Scheduler::getFunctionCount(call) == conf.maxWorkersPerFunction);

        // Check calls have been queued
        REQUIRE(redisQueue.listLength(Scheduler::getFunctionQueueName(call)) == nCalls);

        // Now call the function a few more times and check calls are queued but no new bind messages/ workers added
        Scheduler::callFunction(call);
        Scheduler::callFunction(call);
        Scheduler::callFunction(call);

        REQUIRE(redisQueue.listLength(expectedPrewarmQueue) == conf.maxWorkersPerFunction);
        REQUIRE(Scheduler::getFunctionCount(call) == conf.maxWorkersPerFunction);
        REQUIRE(redisQueue.listLength(Scheduler::getFunctionQueueName(call)) == nCalls + 3);
    }
}
