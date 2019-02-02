#include <catch/catch.hpp>

#include "utils.h"

#include <util/environment.h>
#include <scheduler/Scheduler.h>
#include <redis/Redis.h>

using namespace scheduler;
using namespace redis;

namespace tests {
    static Scheduler &sch = scheduler::getScheduler();
    static Redis &redis = Redis::getQueue();

    static std::string thisHost = util::getEnvVar("HOSTNAME", "");
    static std::string otherHostA = "Host A";
    static std::string otherHostB = "Host B";

    message::Message getCall() {
        message::Message call;
        call.set_user("user a");
        call.set_function("function a");

        return call;
    }

    static void setUp() {
        cleanSystem();
    }

    TEST_CASE("Test worker finishing and host removal", "[scheduler]") {
        setUp();
        message::Message call = getCall();

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
        setUp();
        message::Message call = getCall();

        REQUIRE(sch.getFunctionQueueLength(call) == 0);
        REQUIRE(sch.getFunctionThreadCount(call) == 0);

        // Call the function
        sch.callFunction(call);

        // Check function count has increased and bind message sent
        REQUIRE(sch.getFunctionQueueLength(call) == 1);
        REQUIRE(sch.getBindQueue()->size() == 1);
        message::Message actual = sch.getBindQueue()->dequeue();

        REQUIRE(actual.user() == "user a");
        REQUIRE(actual.function() == "function a");

        redis.flushAll();
    }

    TEST_CASE("Test sending bind messages", "[scheduler]") {
        setUp();
        message::Message call = getCall();

        // Set up calls
        message::Message callA;
        callA.set_user("demo");
        callA.set_function("echo");

        message::Message callB;
        callB.set_user("demo");
        callB.set_function("x2");

        // Add a worker host to the global set
        redis.sadd(GLOBAL_WORKER_SET, "foo");

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
        InMemoryMessageQueue *bindQueue = sch.getBindQueue();
        message::Message bindA = bindQueue->dequeue();
        message::Message bindB = bindQueue->dequeue();

        REQUIRE(bindA.user() == callA.user());
        REQUIRE(bindA.function() == callA.function());
        REQUIRE(bindA.type() == message::Message_MessageType_BIND);

        REQUIRE(bindB.user() == callB.user());
        REQUIRE(bindB.function() == callB.function());
        REQUIRE(bindB.type() == message::Message_MessageType_BIND);

        redis.flushAll();
    }

    TEST_CASE("Test calling function with existing workers does not send bind message", "[scheduler]") {
        setUp();
        message::Message call = getCall();

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

        redis.flushAll();
    }

    TEST_CASE("Test calling function which breaches queue ratio sends bind message", "[scheduler]") {
        setUp();
        message::Message call = getCall();

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

        redis.flushAll();
    }

    TEST_CASE("Test function which breaches queue ratio but has no capacity fails over", "[scheduler]") {
        setUp();
        message::Message call = getCall();

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

        // Check "best host" is now different
        REQUIRE(sch.getBestHostForFunction(call) != thisHost);

        // Now call the function a few more times and check calls are queued but no new bind messages/ workers
        // added to this host
        sch.callFunction(call);
        sch.callFunction(call);
        sch.callFunction(call);

        REQUIRE(bindQueue->size() == conf.maxWorkersPerFunction);
        REQUIRE(sch.getFunctionThreadCount(call) == conf.maxWorkersPerFunction);
        REQUIRE(sch.getFunctionQueueLength(call) == nCalls + 3);

        redis.flushAll();
    }

    TEST_CASE("Test scheduler adds hostname to global set when starting up", "[scheduler]") {
        setUp();

        std::string thisHostname = util::getEnvVar("HOSTNAME", "");
        REQUIRE(!thisHostname.empty());
        REQUIRE(redis.sismember(GLOBAL_WORKER_SET, thisHostname));
    }

    void setUpHostChoiceCheck() {
        setUp();

        redis.sadd(GLOBAL_WORKER_SET, otherHostA);
        redis.sadd(GLOBAL_WORKER_SET, otherHostB);
    }

    TEST_CASE("Test current host chosen when no warm alternatives", "[scheduler]") {
        setUpHostChoiceCheck();
        message::Message call = getCall();

        REQUIRE(sch.getBestHostForFunction(call) == thisHost);
    }

    TEST_CASE("Test other warm option chosen when available", "[scheduler]") {
        setUpHostChoiceCheck();
        message::Message call = getCall();

        // Add another host to the warm set for the given function
        const std::string warmSet = sch.getFunctionWarmSetName(call);
        redis.sadd(warmSet, otherHostA);

        REQUIRE(sch.getBestHostForFunction(call) == otherHostA);
    }

    TEST_CASE("Test current host chosen when already warm even if alternatives", "[scheduler]") {
        setUpHostChoiceCheck();
        message::Message call = getCall();

        // Ensure a warm worker exists on this host
        sch.callFunction(call);
        REQUIRE(sch.getFunctionThreadCount(call) == 1);

        // Check this host is in the warm set
        const std::string warmSet = sch.getFunctionWarmSetName(call);
        REQUIRE(redis.sismember(warmSet, thisHost));

        // Add another host to the warm set
        redis.sadd(warmSet, otherHostA);

        // Run a few times to make sure
        REQUIRE(sch.getBestHostForFunction(call) == thisHost);
        REQUIRE(sch.getBestHostForFunction(call) == thisHost);
        REQUIRE(sch.getBestHostForFunction(call) == thisHost);
        REQUIRE(sch.getBestHostForFunction(call) == thisHost);
    }

}
