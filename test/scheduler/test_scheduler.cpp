#include <catch/catch.hpp>

#include "utils.h"

#include <scheduler/scheduler.h>
#include <redis/redis.h>
#include <util/util.h>

using namespace scheduler;
using namespace redis;

namespace tests {
    TEST_CASE("Test worker finishing and host removal", "[scheduler]") {
        cleanSystem();
        scheduler::Scheduler &sch = scheduler::getScheduler();
        Redis &redis = Redis::getQueue();

        message::Message call;
        call.set_user("userA");
        call.set_function("funcA");

        // Sanity check to see that the host is in the global set but not the set for the function
        std::string hostname = util::getHostName();
        std::string funcSet = sch.getFunctionWarmSetName(call);
        REQUIRE(redis.sismember(GLOBAL_WORKER_SET, hostname));
        REQUIRE(!redis.sismember(funcSet, hostname));

        // Call the function enough to get multiple workers set up
        util::SystemConfig &conf = util::getSystemConfig();
        int requiredCalls = conf.maxQueueRatio * 2;
        for (int i = 0; i < requiredCalls; i++) {
            sch.callFunction(call);
        }

        // Check host is now part of function's set
        REQUIRE(redis.sismember(funcSet, hostname));
        REQUIRE(sch.getFunctionQueueLength(call) == requiredCalls);
        REQUIRE(sch.getBindQueue()->size() == 2);

        // Notify that a worker has finished, count decremented by one and worker is still member of function set
        sch.stopListeningToQueue(call);
        REQUIRE(redis.sismember(funcSet, hostname));
        REQUIRE(sch.getFunctionQueueLength(call) == requiredCalls);
        REQUIRE(sch.getFunctionThreadCount(call) == 1);

        // Notify that another worker has finished, check count is decremented and host removed from function set
        // (but still in global set)
        sch.stopListeningToQueue(call);
        REQUIRE(redis.sismember(GLOBAL_WORKER_SET, hostname));
        REQUIRE(!redis.sismember(funcSet, hostname));
        REQUIRE(sch.getFunctionQueueLength(call) == requiredCalls);
        REQUIRE(sch.getFunctionThreadCount(call) == 0);
    }

    TEST_CASE("Test calling function with no workers sends bind message", "[scheduler]") {
        Redis &redisQueue = redis::Redis::getQueue();
        redisQueue.flushAll();
        scheduler::Scheduler &sch = scheduler::getScheduler();

        message::Message call;
        call.set_function("my func");
        call.set_user("some user");

        REQUIRE(sch.getFunctionQueueLength(call) == 0);
        REQUIRE(sch.getFunctionThreadCount(call) == 0);

        // Add a worker host to the global set
        redisQueue.sadd(GLOBAL_WORKER_SET, "foo");
        
        // Call the function
        sch.callFunction(call);

        // Check function count has increased and bind message sent
        REQUIRE(sch.getFunctionQueueLength(call) == 1);
        REQUIRE(sch.getBindQueue()->size() == 1);
        message::Message actual = sch.getBindQueue()->dequeue();

        REQUIRE(actual.function() == "my func");
        REQUIRE(actual.user() == "some user");

        redisQueue.flushAll();
    }

    TEST_CASE("Test sending bind messages", "[scheduler]") {
        Redis &redisQueue = redis::Redis::getQueue();
        redisQueue.flushAll();
        scheduler::Scheduler &sch = scheduler::getScheduler();

        // Set up calls
        message::Message callA;
        callA.set_user("userA");
        callA.set_function("funcA");

        message::Message callB;
        callB.set_user("userB");
        callB.set_function("funcB");

        // Add a worker host to the global set
        redisQueue.sadd(GLOBAL_WORKER_SET, "foo");
        
        // Call each function
        sch.callFunction(callA);
        sch.callFunction(callB);

        // Check that calls are queued
        REQUIRE(sch.getFunctionQueueLength(callA) == 1);
        REQUIRE(sch.getFunctionThreadCount(callA) == 1);
        REQUIRE(sch.getFunctionQueueLength(callB) == 1);
        REQUIRE(sch.getFunctionThreadCount(callB) == 1);
        REQUIRE(sch.getBindQueue()->size() == 2);

        // Check that bind messages have been sent
        MessageQueue &globalQueue = scheduler::MessageQueue::getGlobalQueue();
        const message::Message bindA = globalQueue.nextMessage();
        const message::Message bindB = globalQueue.nextMessage();

        REQUIRE(bindA.user() == callA.user());
        REQUIRE(bindA.function() == callA.function());
        REQUIRE(bindA.type() == message::Message_MessageType_BIND);

        REQUIRE(bindB.user() == callB.user());
        REQUIRE(bindB.function() == callB.function());
        REQUIRE(bindB.type() == message::Message_MessageType_BIND);

        redisQueue.flushAll();
    }

    TEST_CASE("Test calling function with existing workers does not send bind message", "[scheduler]") {
        Redis &redisQueue = redis::Redis::getQueue();
        redisQueue.flushAll();
        scheduler::Scheduler &sch = scheduler::getScheduler();

        message::Message call;
        call.set_function("my func");
        call.set_user("some user");

        // Add a worker host to the global set
        redisQueue.sadd(GLOBAL_WORKER_SET, "foo");

        // Call the function and check thread count is incremented while bind message sent
        sch.callFunction(call);
        REQUIRE(sch.getFunctionQueueLength(call) == 1);
        REQUIRE(sch.getFunctionThreadCount(call) == 1);
        REQUIRE(sch.getBindQueue()->size() == 1);

        // Call the function again
        sch.callFunction(call);

        // Check function call has been added, but no new bind messages
        REQUIRE(sch.getFunctionQueueLength(call) == 2);
        REQUIRE(sch.getFunctionThreadCount(call) == 1);
        REQUIRE(sch.getBindQueue()->size() == 1);

        redisQueue.flushAll();
    }

    TEST_CASE("Test calling function which breaches queue ratio sends bind message", "[scheduler]") {
        Redis &redisQueue = redis::Redis::getQueue();
        redisQueue.flushAll();
        scheduler::Scheduler &sch = scheduler::getScheduler();

        message::Message call;
        call.set_function("my func");
        call.set_user("some user");

        // Add a worker host to the global set
        redisQueue.sadd(GLOBAL_WORKER_SET, "foo");

        // Saturate up to the number of max queued calls
        util::SystemConfig &conf = util::getSystemConfig();
        int nCalls = conf.maxQueueRatio;
        for (int i = 0; i < nCalls; i++) {
            sch.callFunction(call);

            // Check only one bind message is ever sent
            REQUIRE(redisQueue.listLength(expectedPrewarmQueue) == 1);

            // Check call queued
            REQUIRE(redisQueue.listLength(queueName) == i + 1);
        }

        // Dispatch another and check that a bind message is sent
        sch.callFunction(call);
        REQUIRE(redisQueue.listLength(expectedPrewarmQueue) == 2);
        REQUIRE(redisQueue.listLength(queueName) == nCalls + 1);

        redisQueue.flushAll();
    }

    TEST_CASE("Test function which breaches queue ratio but has no capacity does not scale up", "[scheduler]") {
        Redis &redisQueue = redis::Redis::getQueue();
        redisQueue.flushAll();
        scheduler::Scheduler &sch = scheduler::getScheduler();

        message::Message call;
        call.set_user("userA");
        call.set_function("funcA");

        std::string queueName = sch.getFunctionQueueName(call);

        // Add a worker host to the global set
        redisQueue.sadd(GLOBAL_WORKER_SET, "foo");
        std::string expectedPrewarmQueue = sch.getHostPrewarmQueue("foo");

        // Make calls up to the limit
        util::SystemConfig &conf = util::getSystemConfig();
        int nCalls = conf.maxWorkersPerFunction * conf.maxQueueRatio;
        for (int i = 0; i < nCalls; i++) {
            sch.callFunction(call);
        }

        // Check number of bind messages
        REQUIRE(redisQueue.listLength(expectedPrewarmQueue) == conf.maxWorkersPerFunction);

        // Check count assigned to function
        REQUIRE(sch.getFunctionCount(call) == conf.maxWorkersPerFunction);

        // Check calls have been queued
        REQUIRE(redisQueue.listLength(sch.getFunctionQueueName(call)) == nCalls);

        // Now call the function a few more times and check calls are queued but no new bind messages/ workers added
        sch.callFunction(call);
        sch.callFunction(call);
        sch.callFunction(call);

        REQUIRE(redisQueue.listLength(expectedPrewarmQueue) == conf.maxWorkersPerFunction);
        REQUIRE(sch.getFunctionCount(call) == conf.maxWorkersPerFunction);
        REQUIRE(redisQueue.listLength(sch.getFunctionQueueName(call)) == nCalls + 3);

        redisQueue.flushAll();
    }

    void checkPrewarmQueueChoice(bool affinity) {
        Redis &redisQueue = redis::Redis::getQueue();
        redisQueue.flushAll();
        scheduler::Scheduler &sch = scheduler::getScheduler();

        std::string hostA = "hostA";
        std::string hostB = "hostB";
        std::string hostC = "hostC";
        std::string hostD = "hostD";

        const std::string prewarmA = sch.getHostPrewarmQueue(hostA);
        const std::string prewarmB = sch.getHostPrewarmQueue(hostB);
        const std::string prewarmC = sch.getHostPrewarmQueue(hostC);
        const std::string prewarmD = sch.getHostPrewarmQueue(hostD);

        message::Message msg;
        msg.set_user("alpha");
        msg.set_function("beta");

        std::string workerSet = sch.getFunctionWarmSetName(msg);

        // Add workers to the global set
        redisQueue.sadd(GLOBAL_WORKER_SET, hostA);
        redisQueue.sadd(GLOBAL_WORKER_SET, hostB);

        // With no workers assigned to function, should just get one of the global set
        // regardless of affinity
        std::string actual = sch.getBestHostForFunction(msg, affinity);
        std::set<std::string> expected = {prewarmA, prewarmB};
        REQUIRE(expected.find(actual) != expected.end());

        // Work out which host it is
        std::string chosenHost;
        std::string otherHost;
        std::string otherQueue;
        if (actual == prewarmA) {
            chosenHost = hostA;
            otherHost = hostB;
            otherQueue = prewarmB;
        } else {
            chosenHost = hostB;
            otherHost = hostA;
            otherQueue = prewarmA;
        }

        // Check that this is now added to the function set
        REQUIRE(redisQueue.sismember(workerSet, chosenHost));

        // Run again and check result depending on affinity
        std::string actual2 = sch.getBestHostForFunction(msg, affinity);
        if (affinity) {
            // If affinity, should be the one we already have
            REQUIRE(actual2 == actual);

            // Now remove it from the global worker set and check we get the other
            redisQueue.srem(GLOBAL_WORKER_SET, chosenHost);
            REQUIRE(sch.getBestHostForFunction(msg, affinity) == otherQueue);
            REQUIRE(redisQueue.sismember(workerSet, otherHost));
        } else {
            // If anti-affinity should be the other from the global set
            REQUIRE(actual2 == otherQueue);

            // Now remove everything from the global worker set and leave only one host
            // in the function's set, this should force it to pick the same one
            redisQueue.del(GLOBAL_WORKER_SET);
            redisQueue.srem(workerSet, otherHost);

            REQUIRE(sch.getBestHostForFunction(msg, affinity) == actual);
            REQUIRE(redisQueue.sismember(workerSet, chosenHost));
        }

        redisQueue.flushAll();
    }

    TEST_CASE("Test getting prewarm queue for function with affinity", "[scheduler]") {
        checkPrewarmQueueChoice(true);
    }

    TEST_CASE("Test getting prewarm queue for function with anti-affinity", "[scheduler]") {
        checkPrewarmQueueChoice(false);
    }

    TEST_CASE("Test error is thrown when no scheduling options", "[scheduler]") {
        Redis &redisQueue = redis::Redis::getQueue();
        redisQueue.flushAll();
        scheduler::Scheduler &sch = scheduler::getScheduler();

        message::Message msg;
        msg.set_user("alpha");
        msg.set_function("beta");

        REQUIRE_THROWS(sch.getBestHostForFunction(msg, true));
    }

    TEST_CASE("Test listening/ stopping listening on queue map", "[scheduler]") {
        LocalQueueMap &queueMap = LocalQueueMap::getInstance();
        queueMap.clear();

        message::Message msg;
        msg.set_user("user a");
        msg.set_function("func a");

        REQUIRE(queueMap.getFunctionQueueLength(msg) == 0);
        REQUIRE(queueMap.getFunctionThreadCount(msg) == 0);

        // Listen to the queue for this function
        queueMap.listenToQueue(msg);

        REQUIRE(queueMap.getFunctionQueueLength(msg) == 0);
        REQUIRE(queueMap.getFunctionThreadCount(msg) == 1);

        // Stop listening
        queueMap.stopListeningToQueue(msg);

        REQUIRE(queueMap.getFunctionQueueLength(msg) == 0);
        REQUIRE(queueMap.getFunctionThreadCount(msg) == 0);
    }

    TEST_CASE("Test enqueueing messages") {
        LocalQueueMap &queueMap = LocalQueueMap::getInstance();
        queueMap.clear();

        message::Message bindMsg;
        bindMsg.set_user("user a");
        bindMsg.set_function("func a");
        bindMsg.set_type(message::Message_MessageType_BIND);

        message::Message callMsg;
        callMsg.set_user("user a");
        callMsg.set_function("func a");
        callMsg.set_type(message::Message_MessageType_CALL);

        queueMap.enqueueMessage(bindMsg);
        queueMap.enqueueMessage(callMsg);

        InMemoryMessageQueue *funcQueue = queueMap.getFunctionQueue(callMsg);
        InMemoryMessageQueue *bindQueue = queueMap.getBindQueue();

        REQUIRE(funcQueue->size() == 1);
        REQUIRE(bindQueue->size() == 1);

        message::Message actualCall = funcQueue->dequeue();
        message::Message actualBind = bindQueue->dequeue();

        REQUIRE(actualCall.type() == message::Message_MessageType_CALL);
        REQUIRE(actualBind.type() == message::Message_MessageType_BIND);
    }

    // TODO - check that best host for function updates with changes in queueing etc.
}
