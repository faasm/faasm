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
        InMemoryMessageQueue *bindQueue = sch.getBindQueue();
        int nCalls = conf.maxQueueRatio;
        for (int i = 0; i < nCalls; i++) {
            sch.callFunction(call);

            // Check only one bind message is ever sent
            REQUIRE(bindQueue->size() == 1);

            // Check call queued
            REQUIRE(sch.getFunctionQueueLength(call) == i + 1);
        }

        // Dispatch another and check that a bind message is sent
        sch.callFunction(call);
        REQUIRE(bindQueue->size() == 2);
        REQUIRE(sch.getFunctionQueueLength(call) == nCalls + 1);

        redisQueue.flushAll();
    }

    TEST_CASE("Test function which breaches queue ratio but has no capacity does not scale up", "[scheduler]") {
        Redis &redisQueue = redis::Redis::getQueue();
        redisQueue.flushAll();
        scheduler::Scheduler &sch = scheduler::getScheduler();

        message::Message call;
        call.set_user("userA");
        call.set_function("funcA");
        
        // Add a worker host to the global set
        redisQueue.sadd(GLOBAL_WORKER_SET, "foo");

        // Make calls up to the limit
        util::SystemConfig &conf = util::getSystemConfig();
        int nCalls = conf.maxWorkersPerFunction * conf.maxQueueRatio;
        for (int i = 0; i < nCalls; i++) {
            sch.callFunction(call);
        }

        // Check number of bind messages
        InMemoryMessageQueue *bindQueue = sch.getBindQueue();
        REQUIRE(bindQueue->size() == conf.maxWorkersPerFunction);
        REQUIRE(sch.getFunctionThreadCount(call) == conf.maxWorkersPerFunction);

        // Check calls have been queued
        REQUIRE(sch.getFunctionQueueLength(call) == nCalls);

        // Now call the function a few more times and check calls are queued but no new bind messages/ workers added
        sch.callFunction(call);
        sch.callFunction(call);
        sch.callFunction(call);

        REQUIRE(bindQueue->size() == conf.maxWorkersPerFunction);
        REQUIRE(sch.getFunctionThreadCount(call) == conf.maxWorkersPerFunction);
        REQUIRE(sch.getFunctionQueueLength(call) == nCalls + 3);

        redisQueue.flushAll();
    }

    // TODO - check that best host for function updates with changes in queueing etc.

    TEST_CASE("Test error is thrown when no scheduling options", "[scheduler]") {
        Redis &redisQueue = redis::Redis::getQueue();
        redisQueue.flushAll();
        scheduler::Scheduler &sch = scheduler::getScheduler();

        message::Message msg;
        msg.set_user("alpha");
        msg.set_function("beta");

        REQUIRE_THROWS(sch.getBestHostForFunction(msg));
    }
}
