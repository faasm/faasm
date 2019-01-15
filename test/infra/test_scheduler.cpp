#include <catch/catch.hpp>
#include <infra/infra.h>
#include <util/util.h>
#include "utils.h"

using namespace infra;

namespace tests {
    TEST_CASE("Test worker finishing and host removal", "[scheduler]") {
        cleanSystem();
        infra::Scheduler &sch = infra::getScheduler();
        Redis &redis = Redis::getQueue();

        message::Message call;
        call.set_user("userA");
        call.set_function("funcA");

        // Sanity check to see that the host is in the global set but not the set for the function
        std::string hostname = util::getHostName();
        std::string funcSet = sch.getFunctionWorkerSetName(call);
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
        REQUIRE(sch.getFunctionCount(call) == 2);

        // Fake two workers binding
        sch.workerBound(call);
        sch.workerBound(call);

        // Notify that a worker has finished, count decremented by one and worker is still member of function set
        sch.workerUnbound(call);
        REQUIRE(redis.sismember(funcSet, hostname));
        REQUIRE(sch.getFunctionQueueLength(call) == requiredCalls);
        REQUIRE(sch.getFunctionCount(call) == 1);

        // Notify that another worker has finished, check count is decremented and host removed from function set
        // (but still in global set)
        sch.workerUnbound(call);
        REQUIRE(redis.sismember(GLOBAL_WORKER_SET, hostname));
        REQUIRE(!redis.sismember(funcSet, hostname));
        REQUIRE(sch.getFunctionQueueLength(call) == requiredCalls);
        REQUIRE(sch.getFunctionCount(call) == 0);
    }

    TEST_CASE("Test calling function with no workers sends bind message", "[scheduler]") {
        Redis &redisQueue = infra::Redis::getQueue();
        redisQueue.flushAll();
        infra::Scheduler &sch = infra::getScheduler();

        message::Message call;
        call.set_function("my func");
        call.set_user("some user");

        REQUIRE(sch.getFunctionCount(call) == 0);

        // Add a worker host to the global set
        redisQueue.sadd(GLOBAL_WORKER_SET, "foo");
        std::string expectedPrewarmQueue = sch.getHostPrewarmQueue("foo");

        // Call the function
        sch.callFunction(call);

        // Check function count has increased and bind message sent
        REQUIRE(sch.getFunctionCount(call) == 1);

        REQUIRE(redisQueue.listLength(expectedPrewarmQueue) == 1);
        redisQueue.flushAll();
    }

    TEST_CASE("Test sending bind messages", "[scheduler]") {
        Redis &redisQueue = infra::Redis::getQueue();
        redisQueue.flushAll();
        infra::Scheduler &sch = infra::getScheduler();

        // Set up calls
        message::Message callA;
        callA.set_user("userA");
        callA.set_function("funcA");

        message::Message callB;
        callB.set_user("userB");
        callB.set_function("funcB");

        // Add a worker host to the global set
        redisQueue.sadd(GLOBAL_WORKER_SET, "foo");
        std::string expectedPrewarmQueue = sch.getHostPrewarmQueue("foo");

        // Call each function
        sch.callFunction(callA);
        sch.callFunction(callB);

        // Check that counters are updated (should be done before bind is dispatched)
        REQUIRE(sch.getFunctionCount(callA) == 1);
        REQUIRE(sch.getFunctionCount(callB) == 1);

        // Check that bind messages have been sent
        const message::Message bindA = redisQueue.nextMessage(expectedPrewarmQueue);
        const message::Message bindB = redisQueue.nextMessage(expectedPrewarmQueue);

        REQUIRE(bindA.user() == callA.user());
        REQUIRE(bindA.function() == callA.function());
        REQUIRE(bindA.type() == message::Message_MessageType_BIND);

        REQUIRE(bindB.user() == callB.user());
        REQUIRE(bindB.function() == callB.function());
        REQUIRE(bindB.type() == message::Message_MessageType_BIND);

        redisQueue.flushAll();
    }

    TEST_CASE("Test calling function with existing workers does not send bind message", "[scheduler]") {
        Redis &redisQueue = infra::Redis::getQueue();
        redisQueue.flushAll();
        infra::Scheduler &sch = infra::getScheduler();

        message::Message call;
        call.set_function("my func");
        call.set_user("some user");

        // Add a worker host to the global set
        redisQueue.sadd(GLOBAL_WORKER_SET, "foo");
        std::string expectedPrewarmQueue = sch.getHostPrewarmQueue("foo");

        std::string queueName = sch.getFunctionQueueName(call);

        // Fake calling the function first, should cause a bind message and set up the function queue
        sch.callFunction(call);
        REQUIRE(redisQueue.listLength(expectedPrewarmQueue) == 1);
        REQUIRE(redisQueue.listLength(queueName) == 1);

        // Call the function again
        sch.callFunction(call);

        // Check function call has been added, but no new bind messages
        REQUIRE(redisQueue.listLength(expectedPrewarmQueue) == 1);
        REQUIRE(redisQueue.listLength(queueName) == 2);

        redisQueue.flushAll();
    }

    TEST_CASE("Test calling function which breaches queue ratio sends bind message", "[scheduler]") {
        Redis &redisQueue = infra::Redis::getQueue();
        redisQueue.flushAll();
        infra::Scheduler &sch = infra::getScheduler();

        message::Message call;
        call.set_function("my func");
        call.set_user("some user");

        std::string queueName = sch.getFunctionQueueName(call);

        // Add a worker host to the global set
        redisQueue.sadd(GLOBAL_WORKER_SET, "foo");
        std::string expectedPrewarmQueue = sch.getHostPrewarmQueue("foo");

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
        Redis &redisQueue = infra::Redis::getQueue();
        redisQueue.flushAll();
        infra::Scheduler &sch = infra::getScheduler();

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
        Redis &redisQueue = infra::Redis::getQueue();
        redisQueue.flushAll();
        infra::Scheduler &sch = infra::getScheduler();

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

        std::string workerSet = sch.getFunctionWorkerSetName(msg);

        // Add workers to the global set
        redisQueue.sadd(GLOBAL_WORKER_SET, hostA);
        redisQueue.sadd(GLOBAL_WORKER_SET, hostB);

        // With no workers assigned to function, should just get one of the global set
        // regardless of affinity
        std::string actual = sch.getPrewarmQueueForFunction(msg, affinity);
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
        std::string actual2 = sch.getPrewarmQueueForFunction(msg, affinity);
        if (affinity) {
            // If affinity, should be the one we already have
            REQUIRE(actual2 == actual);

            // Now remove it from the global worker set and check we get the other
            redisQueue.srem(GLOBAL_WORKER_SET, chosenHost);
            REQUIRE(sch.getPrewarmQueueForFunction(msg, affinity) == otherQueue);
            REQUIRE(redisQueue.sismember(workerSet, otherHost));
        } else {
            // If anti-affinity should be the other from the global set
            REQUIRE(actual2 == otherQueue);

            // Now remove everything from the global worker set and leave only one host
            // in the function's set, this should force it to pick the same one
            redisQueue.del(GLOBAL_WORKER_SET);
            redisQueue.srem(workerSet, otherHost);

            REQUIRE(sch.getPrewarmQueueForFunction(msg, affinity) == actual);
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
        Redis &redisQueue = infra::Redis::getQueue();
        redisQueue.flushAll();
        infra::Scheduler &sch = infra::getScheduler();

        message::Message msg;
        msg.set_user("alpha");
        msg.set_function("beta");

        REQUIRE_THROWS(sch.getPrewarmQueueForFunction(msg, true));
    }
}
