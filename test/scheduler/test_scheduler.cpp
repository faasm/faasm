#include <catch/catch.hpp>

#include "utils.h"

#include <util/environment.h>
#include <scheduler/Scheduler.h>
#include <redis/Redis.h>

using namespace scheduler;
using namespace redis;

namespace tests {
    TEST_CASE("Test scheduler destructor", "[scheduler]") {
        cleanSystem();

        message::Message call;
        call.set_user("some user");
        call.set_function("some function");
        std::string funcSet;

        std::string hostname = util::getHostName();
        Redis &redis = Redis::getQueue();

        // Create a scheduler in its own context
        {
            Scheduler s;

            funcSet = s.getFunctionWarmSetName(call);

            // Check initial set-up
            REQUIRE(redis.sismember(GLOBAL_WORKER_SET, hostname));
            REQUIRE(!redis.sismember(funcSet, hostname));

            // Call the function and check it's added to the function's warm set
            s.callFunction(call);

            REQUIRE(redis.sismember(GLOBAL_WORKER_SET, hostname));
            REQUIRE(redis.sismember(funcSet, hostname));
        }

        // After destructor has run this host should no longer be part of either set
        REQUIRE(!redis.sismember(GLOBAL_WORKER_SET, hostname));
        REQUIRE(!redis.sismember(funcSet, hostname));
    }

    TEST_CASE("Test scheduler operations", "[scheduler]") {
        cleanSystem();

        Scheduler &sch = scheduler::getScheduler();
        Redis &redis = Redis::getQueue();

        std::string hostname = util::getHostName();
        std::string otherHostA = "Host A";
        SharingMessageBus &sharingBus = SharingMessageBus::getInstance();

        message::Message call;
        call.set_user("user a");
        call.set_function("function a");

        util::SystemConfig &conf = util::getSystemConfig();

        SECTION("Test worker finishing and host removal") {
            // Sanity check to see that the host is in the global set but not the set for the function
            std::string funcSet = sch.getFunctionWarmSetName(call);
            REQUIRE(redis.sismember(GLOBAL_WORKER_SET, hostname));
            REQUIRE(!redis.sismember(funcSet, hostname));

            // Call the function enough to get multiple workers set up
            int requiredCalls = conf.maxQueueRatio * 2;
            for (int i = 0; i < requiredCalls; i++) {
                sch.callFunction(call);
            }

            // Check host is now part of function's set
            REQUIRE(redis.sismember(funcSet, hostname));
            REQUIRE(sch.getFunctionQueueLength(call) == requiredCalls);
            REQUIRE(sch.bindQueue.size() == 2);

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

        SECTION("Test calling function with no workers sends bind message") {
            REQUIRE(sch.getFunctionQueueLength(call) == 0);
            REQUIRE(sch.getFunctionThreadCount(call) == 0);

            // Call the function
            sch.callFunction(call);

            // Check function count has increased and bind message sent
            REQUIRE(sch.getFunctionQueueLength(call) == 1);
            REQUIRE(sch.bindQueue.size() == 1);
            message::Message actual = sch.bindQueue.dequeue();

            REQUIRE(actual.user() == "user a");
            REQUIRE(actual.function() == "function a");

            redis.flushAll();
        }

        SECTION("Test sending bind messages") {
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
            REQUIRE(sch.bindQueue.size() == 2);

            // Check that bind messages have been sent
            message::Message bindA = sch.bindQueue.dequeue();
            message::Message bindB = sch.bindQueue.dequeue();

            REQUIRE(bindA.user() == callA.user());
            REQUIRE(bindA.function() == callA.function());
            REQUIRE(bindA.type() == message::Message_MessageType_BIND);

            REQUIRE(bindB.user() == callB.user());
            REQUIRE(bindB.function() == callB.function());
            REQUIRE(bindB.type() == message::Message_MessageType_BIND);

            redis.flushAll();
        }

        SECTION("Test calling function with existing workers does not send bind message") {
            // Call the function and check thread count is incremented while bind message sent
            sch.callFunction(call);
            REQUIRE(sch.getFunctionQueueLength(call) == 1);
            REQUIRE(sch.getFunctionThreadCount(call) == 1);
            REQUIRE(sch.bindQueue.size() == 1);

            // Call the function again
            sch.callFunction(call);

            // Check function call has been added, but no new bind messages
            REQUIRE(sch.getFunctionQueueLength(call) == 2);
            REQUIRE(sch.getFunctionThreadCount(call) == 1);
            REQUIRE(sch.bindQueue.size() == 1);

            redis.flushAll();
        }

        SECTION("Test calling function which breaches queue ratio sends bind message") {
            // Saturate up to the number of max queued calls
            int nCalls = conf.maxQueueRatio;
            for (int i = 0; i < nCalls; i++) {
                sch.callFunction(call);

                // Check only one bind message is ever sent
                REQUIRE(sch.bindQueue.size() == 1);

                // Check call queued
                REQUIRE(sch.getFunctionQueueLength(call) == i + 1);
            }

            // Dispatch another and check that a bind message is sent
            sch.callFunction(call);
            REQUIRE(sch.bindQueue.size() == 2);
            REQUIRE(sch.getFunctionQueueLength(call) == nCalls + 1);

            redis.flushAll();
        }

        SECTION("Host choice checks") {
            redis.sadd(GLOBAL_WORKER_SET, otherHostA);

            SECTION("Test function which breaches queue ratio but has no capacity shares with other host") {
                // Make calls up to the limit
                int nCalls = conf.maxWorkersPerFunction * conf.maxQueueRatio;
                for (int i = 0; i < nCalls; i++) {
                    sch.callFunction(call);
                }

                // Check local workers requested
                REQUIRE(sch.bindQueue.size() == conf.maxWorkersPerFunction);
                REQUIRE(sch.getFunctionThreadCount(call) == conf.maxWorkersPerFunction);

                // Check calls have been queued
                REQUIRE(sch.getFunctionQueueLength(call) == nCalls);

                // Check "best host" is now different
                REQUIRE(sch.getBestHostForFunction(call) != hostname);

                // Call more and check calls are shared elsewhere
                sch.callFunction(call);
                sch.callFunction(call);

                message::Message actualA = sharingBus.nextMessageForHost(otherHostA);
                message::Message actualB = sharingBus.nextMessageForHost(otherHostA);

                REQUIRE(actualA.function() == call.function());
                REQUIRE(actualA.user() == call.user());

                REQUIRE(actualB.function() == call.function());
                REQUIRE(actualB.user() == call.user());

                // Check not added to local queues
                REQUIRE(sch.bindQueue.size() == conf.maxWorkersPerFunction);
                REQUIRE(sch.getFunctionThreadCount(call) == conf.maxWorkersPerFunction);
                REQUIRE(sch.getFunctionQueueLength(call) == nCalls);

                redis.flushAll();
            }

            SECTION("Test scheduler requests scale-out when no options") {
                // Clear out worker set and just add this host
                redis.flushAll();
                redis.sadd(GLOBAL_WORKER_SET, hostname);

                GlobalMessageBus &globalBus = getGlobalMessageBus();
                globalBus.clear();

                REQUIRE(sch.getGlobalSetSize() == 1);
                REQUIRE(globalBus.getScaleoutRequestCount() == 0);

                // Saturate this host
                int nCalls = conf.maxWorkersPerFunction * conf.maxQueueRatio;
                for (int i = 0; i < nCalls; i++) {
                    sch.callFunction(call);
                }

                // Make another request
                sch.callFunction(call);

                // Check scale-out request has been made
                REQUIRE(globalBus.getScaleoutRequestCount() == 1);
            }
            
            SECTION("Test scheduler adds hostname to global set when starting up") {
                REQUIRE(!hostname.empty());
                REQUIRE(redis.sismember(GLOBAL_WORKER_SET, hostname));
            }

            SECTION("Test current host chosen when no warm alternatives") {
                REQUIRE(sch.getBestHostForFunction(call) == hostname);
            }

            SECTION("Test other warm option chosen when available") {
                // Add another host to the warm set for the given function
                const std::string warmSet = sch.getFunctionWarmSetName(call);
                redis.sadd(warmSet, otherHostA);

                REQUIRE(sch.getBestHostForFunction(call) == otherHostA);
            }

            SECTION("Test other warm option *not* chosen when in no scheduler mode") {
                conf.noScheduler = 1;

                // Add another host to the warm set for the given function
                const std::string warmSet = sch.getFunctionWarmSetName(call);
                redis.sadd(warmSet, otherHostA);

                // Check it's ignored
                REQUIRE(sch.getBestHostForFunction(call) == hostname);

                conf.noScheduler = 0;
            }

            SECTION("Test current host chosen when already warm even if alternatives") {
                // Ensure a warm worker exists on this host
                sch.callFunction(call);
                REQUIRE(sch.getFunctionThreadCount(call) == 1);

                // Check this host is in the warm set
                const std::string warmSet = sch.getFunctionWarmSetName(call);
                REQUIRE(redis.sismember(warmSet, hostname));

                // Add another host to the warm set
                redis.sadd(warmSet, otherHostA);

                // Run a few times to make sure
                REQUIRE(sch.getBestHostForFunction(call) == hostname);
                REQUIRE(sch.getBestHostForFunction(call) == hostname);
                REQUIRE(sch.getBestHostForFunction(call) == hostname);
                REQUIRE(sch.getBestHostForFunction(call) == hostname);
            }
        }
    }

    TEST_CASE("Test scheduler execution count", "[scheduler]") {
        cleanSystem();

        Scheduler &sch = scheduler::getScheduler();

        REQUIRE(sch.getExecutingCount() == 0);

        sch.incrementExecutingCount();
        sch.incrementExecutingCount();

        REQUIRE(sch.getExecutingCount() == 2);

        sch.decrementExecutingCount();

        REQUIRE(sch.getExecutingCount() == 1);

        // Try to decrement below zero
        sch.decrementExecutingCount();
        sch.decrementExecutingCount();
        sch.decrementExecutingCount();

        REQUIRE(sch.getExecutingCount() == 0);
    }

    TEST_CASE("Test scaling out", "[scheduler]") {
        cleanSystem();
        Scheduler &sch = scheduler::getScheduler();
        GlobalMessageBus &globalBus = scheduler::getGlobalMessageBus();
        Redis &redis = redis::Redis::getQueue();
        util::SystemConfig &conf = util::getSystemConfig();

        // Check there are no workers and no scale-out requests initially
        redis.flushAll();
        REQUIRE(sch.getGlobalSetSize() == 0);
        REQUIRE(globalBus.getScaleoutRequestCount() == 0);

        SECTION("Check scale out requested when none preset") {
            // Request a scale-out (should always dispatch only one request at a time regardless of the target)
            sch.scaleOut(4);

            REQUIRE(globalBus.getScaleoutRequestCount() == 1);
        }

        SECTION("Check scale out ignored when already over target") {
            // Add some random workers
            redis.sadd(GLOBAL_WORKER_SET, "foo");
            redis.sadd(GLOBAL_WORKER_SET, "bar");

            // Request a scale-out (should always dispatch only one request at a time regardless of the target)
            sch.scaleOut(2);

            REQUIRE(globalBus.getScaleoutRequestCount() == 0);
        }

        SECTION("Check scale out ignored when at max") {
            // Add workers up to the limit
            for (int i = 0; i < conf.maxNodes; i++) {
                std::string workerName = "foo" + std::to_string(i);
                redis.sadd(GLOBAL_WORKER_SET, workerName);
            }

            // Request a scale-out over the max
            sch.scaleOut(conf.maxNodes + 1);

            REQUIRE(globalBus.getScaleoutRequestCount() == 0);
        }
    }
}
