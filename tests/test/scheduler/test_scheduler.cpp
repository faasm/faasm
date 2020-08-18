#include <catch/catch.hpp>

#include "utils.h"

#include <util/environment.h>
#include <scheduler/Scheduler.h>
#include <redis/Redis.h>

using namespace scheduler;
using namespace redis;

namespace tests {
    TEST_CASE("Test scheduler clear-up", "[scheduler]") {
        cleanSystem();

        message::Message call;
        call.set_user("some user");
        call.set_function("some function");
        std::string funcSet;

        std::string thisHost = util::getSystemConfig().endpointHost;
        Redis &redis = Redis::getQueue();

        Scheduler s;
        funcSet = s.getFunctionWarmSetName(call);

        // Check initial set-up
        REQUIRE(redis.sismember(AVAILABLE_HOST_SET, thisHost));
        REQUIRE(!redis.sismember(funcSet, thisHost));

        // Call the function and check it's added to the function's warm set
        s.callFunction(call);

        REQUIRE(redis.sismember(AVAILABLE_HOST_SET, thisHost));
        REQUIRE(redis.sismember(funcSet, thisHost));

        // Run clear-up
        s.clear();

        // After clear-up has run this host should no longer be part of either set
        REQUIRE(!redis.sismember(AVAILABLE_HOST_SET, thisHost));
        REQUIRE(!redis.sismember(funcSet, thisHost));
    }

    TEST_CASE("Test scheduler operations", "[scheduler]") {
        cleanSystem();

        Scheduler &sch = scheduler::getScheduler();
        Redis &redis = Redis::getQueue();

        std::string thisHost = util::getSystemConfig().endpointHost;
        std::string otherHostA = "192.168.0.10";
        SharingMessageBus &sharingBus = SharingMessageBus::getInstance();

        message::Message call = util::messageFactory("user a", "function a");
        message::Message chainedCall = util::messageFactory("user a", "function a");
        chainedCall.set_idx(3);

        util::SystemConfig &conf = util::getSystemConfig();
        int originalMaxInFlightRatio = conf.maxInFlightRatio;
        conf.maxInFlightRatio = 8;

        SECTION("Test faaslet finishing and host removal") {
            // Sanity check to see that the host is in the global set but not the set for the function
            std::string funcSet = sch.getFunctionWarmSetName(call);
            REQUIRE(redis.sismember(AVAILABLE_HOST_SET, thisHost));
            REQUIRE(!redis.sismember(funcSet, thisHost));

            // Call the function enough to get multiple faaslets set up
            int requiredCalls = (conf.maxInFlightRatio * 2) - 3;
            for (int i = 0; i < requiredCalls; i++) {
                sch.callFunction(call);
            }

            // Check host is now part of function's set
            REQUIRE(redis.sismember(funcSet, thisHost));
            REQUIRE(sch.getFunctionInFlightCount(call) == requiredCalls);
            REQUIRE(sch.getBindQueue()->size() == 2);

            // Notify that a faaslet has finished. Check that the  count decremented by one and faaslet is still member of function set
            sch.notifyFaasletFinished(call);
            REQUIRE(redis.sismember(funcSet, thisHost));
            REQUIRE(sch.getFunctionInFlightCount(call) == requiredCalls);
            REQUIRE(sch.getFunctionWarmFaasletCount(call) == 1);

            // Notify that another faaslet has finished, check count is decremented and host removed from function set
            // (but still in global set)
            sch.notifyFaasletFinished(call);
            REQUIRE(redis.sismember(AVAILABLE_HOST_SET, thisHost));
            REQUIRE(!redis.sismember(funcSet, thisHost));
            REQUIRE(sch.getFunctionInFlightCount(call) == requiredCalls);
            REQUIRE(sch.getFunctionWarmFaasletCount(call) == 0);
        }

        SECTION("Test calling function with no faaslets sends bind message") {
            REQUIRE(sch.getFunctionInFlightCount(call) == 0);
            REQUIRE(sch.getFunctionWarmFaasletCount(call) == 0);

            // Call the function
            sch.callFunction(call);

            // Check function count has increased and bind message sent
            REQUIRE(sch.getFunctionInFlightCount(call) == 1);
            REQUIRE(sch.getBindQueue()->size() == 1);
            message::Message actual = sch.getBindQueue()->dequeue();

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

            // Add a faaslet host to the global set
            redis.sadd(AVAILABLE_HOST_SET, "192.168.3.45");

            // Call each function
            sch.callFunction(callA);
            sch.callFunction(callB);

            // Check that calls are queued
            REQUIRE(sch.getFunctionInFlightCount(callA) == 1);
            REQUIRE(sch.getFunctionWarmFaasletCount(callA) == 1);
            REQUIRE(sch.getFunctionInFlightCount(callB) == 1);
            REQUIRE(sch.getFunctionWarmFaasletCount(callB) == 1);
            REQUIRE(sch.getBindQueue()->size() == 2);

            // Check that bind messages have been sent
            auto bindQueue = sch.getBindQueue();
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

        SECTION("Test calling function with existing faaslets does not send bind message") {
            // Call the function and check faaslet count is incremented while bind message sent
            sch.callFunction(call);
            REQUIRE(sch.getFunctionInFlightCount(call) == 1);
            REQUIRE(sch.getFunctionWarmFaasletCount(call) == 1);
            REQUIRE(sch.getBindQueue()->size() == 1);

            // Call the function again
            sch.callFunction(call);

            // Check function call has been added, but no new bind messages
            REQUIRE(sch.getFunctionInFlightCount(call) == 2);
            REQUIRE(sch.getFunctionWarmFaasletCount(call) == 1);
            REQUIRE(sch.getBindQueue()->size() == 1);

            redis.flushAll();
        }

        SECTION("Test calling function which breaches in-flight ratio sends bind message") {
            // Saturate up to just below the max in flight
            auto bindQueue = sch.getBindQueue();
            int nCalls = conf.maxInFlightRatio;
            for (int i = 0; i < nCalls; i++) {
                sch.callFunction(call);

                // Check only one bind message has been sent
                REQUIRE(bindQueue->size() == 1);

                // Check call queued
                REQUIRE(sch.getFunctionInFlightCount(call) == i + 1);
            }

            // Dispatch another and check that a bind message is sent
            sch.callFunction(call);
            REQUIRE(bindQueue->size() == 2);
            REQUIRE(sch.getFunctionInFlightCount(call) == nCalls + 1);

            redis.flushAll();
        }

        SECTION("Test counts can't go below zero") {
            message::Message msg = util::messageFactory("demo", "echo");

            sch.notifyFaasletFinished(msg);
            sch.notifyFaasletFinished(msg);
            sch.notifyFaasletFinished(msg);
            REQUIRE(sch.getFunctionWarmFaasletCount(msg) == 0);

            sch.notifyCallFinished(msg);
            sch.notifyCallFinished(msg);
            sch.notifyCallFinished(msg);
            sch.notifyCallFinished(msg);
            REQUIRE(sch.getFunctionInFlightCount(msg) == 0);
        }

        SECTION("Host choice checks") {
            redis.sadd(AVAILABLE_HOST_SET, otherHostA);

            SECTION("Test function which breaches in-flight ratio but has no capacity fails over") {
                // Make calls up to just below the limit
                int nCalls = (conf.maxFaasletsPerFunction * conf.maxInFlightRatio) - 1;
                for (int i = 0; i < nCalls; i++) {
                    sch.callFunction(call);
                }

                // Check we're still the best option
                REQUIRE(sch.getBestHostForFunction(call) == thisHost);

                // Check local faaslets requested
                auto bindQueue = sch.getBindQueue();
                REQUIRE(bindQueue->size() == conf.maxFaasletsPerFunction);
                REQUIRE(sch.getFunctionWarmFaasletCount(call) == conf.maxFaasletsPerFunction);

                // Check calls have been queued
                REQUIRE(sch.getFunctionInFlightCount(call) == nCalls);

                // Make another call and check best host is now different
                sch.callFunction(call);
                REQUIRE(sch.getBestHostForFunction(call) != thisHost);
                REQUIRE(sch.getFunctionInFlightCount(call) == nCalls + 1);

                // Call more and check calls are shared elsewhere
                sch.callFunction(call);
                sch.callFunction(call);

                const std::string sharingQueue = getSharingQueueNameForHost(otherHostA);
                REQUIRE(redis.listLength(sharingQueue) == 2);

                message::Message actualA = sharingBus.nextMessageForHost(otherHostA);
                message::Message actualB = sharingBus.nextMessageForHost(otherHostA);

                REQUIRE(actualA.function() == call.function());
                REQUIRE(actualA.user() == call.user());

                REQUIRE(actualB.function() == call.function());
                REQUIRE(actualB.user() == call.user());

                // Check not added to local queues
                REQUIRE(bindQueue->size() == conf.maxFaasletsPerFunction);
                REQUIRE(sch.getFunctionWarmFaasletCount(call) == conf.maxFaasletsPerFunction);
                REQUIRE(sch.getFunctionInFlightCount(call) == nCalls + 1);

                redis.flushAll();
            }

            SECTION("Test scheduler adds host ID to global set when starting up") {
                REQUIRE(!thisHost.empty());
                REQUIRE(redis.sismember(AVAILABLE_HOST_SET, thisHost));
            }

            SECTION("Test current host chosen when no warm alternatives") {
                REQUIRE(sch.getBestHostForFunction(call) == thisHost);
            }

            SECTION("Test other warm option chosen when available") {
                // Add another host to the warm set for the given function
                const std::string warmSet = sch.getFunctionWarmSetName(call);
                redis.sadd(warmSet, otherHostA);

                REQUIRE(sch.getBestHostForFunction(call) == otherHostA);
            }

            SECTION("Test this host chosen when scheduler off even though warm option available") {
                // Switch off scheduler
                int originalNoScheduler = conf.noScheduler;
                conf.noScheduler = 1;

                // Add another host to warm
                const std::string warmSet = sch.getFunctionWarmSetName(call);
                redis.sadd(warmSet, otherHostA);

                REQUIRE(sch.getBestHostForFunction(call) == thisHost);

                conf.noScheduler = originalNoScheduler;
            }

            SECTION("Test other warm option *not* chosen when in no scheduler mode") {
                conf.noScheduler = 1;

                // Add another host to the warm set for the given function
                const std::string warmSet = sch.getFunctionWarmSetName(call);
                redis.sadd(warmSet, otherHostA);

                // Check it's ignored
                REQUIRE(sch.getBestHostForFunction(call) == thisHost);

                conf.noScheduler = 0;
            }

            SECTION("Test current host chosen when already warm even if alternatives") {
                // Ensure a warm faaslet exists on this host
                sch.callFunction(call);
                REQUIRE(sch.getFunctionWarmFaasletCount(call) == 1);

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

        conf.maxInFlightRatio = originalMaxInFlightRatio;
    }

    TEST_CASE("Test message recording of scheduling decisions", "[scheduler]") {
        cleanSystem();
        Scheduler &sch = scheduler::getScheduler();
        SharingMessageBus &sharingBus = SharingMessageBus::getInstance();

        std::string thisHostId = util::getSystemConfig().endpointHost;
        std::string otherHostA = "192.168.3.3";

        util::SystemConfig &conf = util::getSystemConfig();
        int maxInFlightRatio = conf.maxInFlightRatio;
        int maxFaaslets = conf.maxFaasletsPerFunction;

        // Add calls to saturate the first host
        int requiredCalls = maxInFlightRatio * maxFaaslets;
        for (int i = 0; i < requiredCalls; i++) {
            message::Message msgA = util::messageFactory("demo", "chain_simple");
            sch.callFunction(msgA);

            // Check scheduling info
            REQUIRE(msgA.scheduledhost() == thisHostId);
            REQUIRE(msgA.hops() == 0);
            REQUIRE(msgA.executedhost().empty());
        }

        // Now add the other host to the warm set and make
        // sure calls are sent there
        message::Message msgB = util::messageFactory("demo", "chain_simple");
        const std::string warmSet = sch.getFunctionWarmSetName(msgB);
        Redis &redis = redis::Redis::getQueue();
        redis.sadd(warmSet, otherHostA);

        for (int i = 0; i < 3; i++) {
            message::Message msgC = util::messageFactory("demo", "chain_simple");
            sch.callFunction(msgC);

            // Check scheduling info
            REQUIRE(msgC.scheduledhost() == otherHostA);
            REQUIRE(msgC.hops() == 1);
            REQUIRE(msgC.executedhost().empty());

            // Check actual message bus
            message::Message actualShare = sharingBus.nextMessageForHost(otherHostA);
        }
    }

    TEST_CASE("Test multiple hops", "[scheduler]") {
        cleanSystem();
        Scheduler &sch = scheduler::getScheduler();

        std::string thisHostId = util::getSystemConfig().endpointHost;
        std::string otherHostA = "192.168.4.5";

        message::Message msg = util::messageFactory("demo", "chain_simple");

        // Add calls to saturate the first host
        util::SystemConfig &conf = util::getSystemConfig();
        int requiredCalls = conf.maxInFlightRatio * conf.maxFaasletsPerFunction;
        for (int i = 0; i < requiredCalls; i++) {
            sch.callFunction(msg);
        }

        // Add other host to warm set
        const std::string warmSet = sch.getFunctionWarmSetName(msg);
        Redis &redis = redis::Redis::getQueue();
        redis.sadd(warmSet, otherHostA);

        // Now create a message that's already got a scheduled host and hops
        message::Message msgWithHops = util::messageFactory("demo", "chain_simple");
        msgWithHops.set_scheduledhost("Some other host");
        msgWithHops.set_hops(5);

        // Schedule the call
        sch.callFunction(msgWithHops);

        // Check host not changes and hops increased
        REQUIRE(msgWithHops.scheduledhost() == "Some other host");
        REQUIRE(msgWithHops.hops() == 6);
    }

    TEST_CASE("Test faaslet removes itself from warm set when sharing") {
        cleanSystem();
        Scheduler &sch = scheduler::getScheduler();

        std::string thisHost = util::getSystemConfig().endpointHost;
        std::string otherHost = "192.168.111.23";
        message::Message msg = util::messageFactory("demo", "chain_simple");

        // Add both to the global set
        Redis &redis = redis::Redis::getQueue();
        redis.sadd(AVAILABLE_HOST_SET, thisHost);
        redis.sadd(AVAILABLE_HOST_SET, otherHost);

        // Add a call and make sure we're in the warm set
        sch.callFunction(msg);
        const std::string warmSetName = sch.getFunctionWarmSetName(msg);
        REQUIRE(redis.sismember(warmSetName, thisHost));

        // Now saturate up to the point we're about to fail over
        util::SystemConfig &conf = util::getSystemConfig();
        int requiredCalls = conf.maxInFlightRatio * conf.maxFaasletsPerFunction - 2;
        for (int i = 0; i < requiredCalls; i++) {
            sch.callFunction(msg);
        }

        // Check we're still the best host
        REQUIRE(sch.getBestHostForFunction(msg) == thisHost);

        // Add another call, check we're no longer the best
        sch.callFunction(msg);
        REQUIRE(sch.getBestHostForFunction(msg) == otherHost);

        // Make another call and check we're no longer in the warm set
        sch.callFunction(msg);
        REQUIRE(!redis.sismember(warmSetName, thisHost));
    }

    TEST_CASE("Test awaiting/ finished awaiting", "[scheduler]") {
        cleanSystem();
        Scheduler &sch = scheduler::getScheduler();
        message::Message msg = util::messageFactory("demo", "chain_simple");

        // Initial conditions
        REQUIRE(sch.getFunctionWarmFaasletCount(msg) == 0);
        REQUIRE(sch.getFunctionInFlightCount(msg) == 0);

        // Check calling function first adds another faaslet
        sch.callFunction(msg);
        REQUIRE(sch.getFunctionWarmFaasletCount(msg) == 1);
        REQUIRE(sch.getFunctionInFlightCount(msg) == 1);
        REQUIRE(sch.getLatestOpinion(msg) == SchedulerOpinion::YES);

//        // Check notifying of awaiting reduces the faaslet and in-flight count
//        sch.notifyAwaiting(msg);
//        REQUIRE(sch.getFunctionWarmFaasletCount(msg) == 0);
//        REQUIRE(sch.getFunctionInFlightCount(msg) == 0);
//        REQUIRE(sch.getLatestOpinion(msg) == SchedulerOpinion::MAYBE);
//
//        // Check notifying of awaiting finished puts this up again
//        sch.notifyFinishedAwaiting(msg);
//        REQUIRE(sch.getFunctionWarmFaasletCount(msg) == 1);
//        REQUIRE(sch.getFunctionInFlightCount(msg) == 1);
//        REQUIRE(sch.getLatestOpinion(msg) == SchedulerOpinion::YES);
    }

    TEST_CASE("Test opinion still YES when nothing in flight", "[scheduler]") {
        cleanSystem();
        Scheduler &sch = scheduler::getScheduler();
        message::Message msg = util::messageFactory("demo", "chain_simple");

        // Check opinion is maybe initially
        REQUIRE(sch.getLatestOpinion(msg) == SchedulerOpinion::MAYBE);
        REQUIRE(sch.getFunctionWarmFaasletCount(msg) == 0);

        // Call one function and make sure opinion is YES
        sch.callFunction(msg);
        REQUIRE(sch.getFunctionWarmFaasletCount(msg) == 1);
        REQUIRE(sch.getFunctionInFlightCount(msg) == 1);
        REQUIRE(sch.getLatestOpinion(msg) == SchedulerOpinion::YES);

        // Notify finished with call
        sch.notifyCallFinished(msg);
        REQUIRE(sch.getFunctionWarmFaasletCount(msg) == 1);
        REQUIRE(sch.getFunctionInFlightCount(msg) == 0);
        REQUIRE(sch.getLatestOpinion(msg) == SchedulerOpinion::YES);
    }

    TEST_CASE("Test opinion still YES when nothing in flight and at max faaslets", "[scheduler]") {
        cleanSystem();
        util::SystemConfig &conf = util::getSystemConfig();
        Scheduler &sch = scheduler::getScheduler();
        message::Message msg = util::messageFactory("demo", "chain_simple");

        // Check opinion is maybe initially
        REQUIRE(sch.getLatestOpinion(msg) == SchedulerOpinion::MAYBE);
        REQUIRE(sch.getFunctionWarmFaasletCount(msg) == 0);

        // Saturate and make sure opinion is NO
        int nCalls = conf.maxFaasletsPerFunction * conf.maxInFlightRatio;
        for (int i = 0; i < nCalls; i++) {
            sch.callFunction(msg);
        }
        REQUIRE(sch.getFunctionWarmFaasletCount(msg) == conf.maxFaasletsPerFunction);
        REQUIRE(sch.getFunctionInFlightCount(msg) == nCalls);
        REQUIRE(sch.getLatestOpinion(msg) == SchedulerOpinion::NO);

        // Notify all calls finished
        for (int i = 0; i < nCalls; i++) {
            sch.notifyCallFinished(msg);
        }
        REQUIRE(sch.getFunctionWarmFaasletCount(msg) == conf.maxFaasletsPerFunction);
        REQUIRE(sch.getFunctionInFlightCount(msg) == 0);
        REQUIRE(sch.getLatestOpinion(msg) == SchedulerOpinion::YES);
    }

    TEST_CASE("Test special case scheduling of MPI functions", "[mpi]") {
        cleanSystem();

        util::SystemConfig &conf = util::getSystemConfig();
        int originalInFlight = conf.maxInFlightRatio;
        int originalFaasletsPerFunc = conf.maxFaasletsPerFunction;

        // Set up known params
        int inFlightRatio = 2;
        int faasletsPerFunc = 4;
        conf.maxInFlightRatio = inFlightRatio;
        conf.maxFaasletsPerFunction = faasletsPerFunc;

        message::Message msg = util::messageFactory("mpi", "hellompi");
        msg.set_ismpi(true);

        Scheduler &sch = getScheduler();

        // Max in-flight ratio should be 1, hence one faaslet created per call
        for (int i = 0; i < inFlightRatio; i++) {
            sch.callFunction(msg);
        }
        REQUIRE(sch.getFunctionWarmFaasletCount(msg) == inFlightRatio);
        REQUIRE(sch.getLatestOpinion(msg) == SchedulerOpinion::YES);

        // Saturate up to max faaslets
        int remainingCalls = faasletsPerFunc - inFlightRatio;
        for (int i = 0; i < remainingCalls; i++) {
            sch.callFunction(msg);
        }

        // Check scheduler no longer accepting calls
        REQUIRE(sch.getFunctionWarmFaasletCount(msg) == faasletsPerFunc);
        REQUIRE(sch.getLatestOpinion(msg) == SchedulerOpinion::NO);

        // Reset conf
        conf.maxInFlightRatio = originalInFlight;
        conf.maxFaasletsPerFunction = originalFaasletsPerFunc;
    }

    TEST_CASE("Test logging message IDs", "[scheduler]") {
        cleanSystem();

        Scheduler &sch = scheduler::getScheduler();

        message::Message msgA = util::messageFactory("demo", "echo");
        message::Message msgB = util::messageFactory("demo", "echo");
        message::Message msgC = util::messageFactory("demo", "echo");

        SECTION("No logging") {
            sch.setRecordKeeping(false);

            sch.callFunction(msgA);
            sch.callFunction(msgB);
            sch.callFunction(msgC);
            REQUIRE(sch.getRecordedMessagesAll().empty());
        }

        SECTION("Logging") {
            sch.setRecordKeeping(true);

            sch.callFunction(msgA);
            sch.callFunction(msgB);
            sch.callFunction(msgC);

            std::vector<unsigned int> expected = {(unsigned int) msgA.id(), (unsigned int) msgB.id(),
                                                  (unsigned int) msgC.id()};
            std::vector<unsigned int> actual = sch.getRecordedMessagesAll();
            REQUIRE(actual == expected);
        }
    }

    TEST_CASE("Test max Faaslet limit with multiple functions", "[scheduler]") {
        cleanSystem();

        Scheduler &sch = scheduler::getScheduler();
        Redis &redis = Redis::getQueue();

        std::string thisHost = sch.getThisHost();
        std::string otherHost = "192.168.0.10";

        message::Message callA = util::messageFactory("user a", "function a1");
        message::Message callB = util::messageFactory("user a", "function a2");
        message::Message callC = util::messageFactory("user b", "function b1");

        util::SystemConfig &conf = util::getSystemConfig();
        int originalMaxInFlightRatio = conf.maxInFlightRatio;
        int originalMaxFaaslet = conf.maxFaaslets;
        int originalMaxFaasletsPerFunction = conf.maxFaasletsPerFunction;

        conf.maxFaaslets = 10;
        conf.maxFaasletsPerFunction = 6;
        conf.maxInFlightRatio = 1;

        // Make other host available
        redis.sadd(AVAILABLE_HOST_SET, otherHost);

        // Check we're a MAYBE for all functions initially
        REQUIRE(sch.getLatestOpinion(callA) == SchedulerOpinion::MAYBE);
        REQUIRE(sch.getLatestOpinion(callB) == SchedulerOpinion::MAYBE);
        REQUIRE(sch.getLatestOpinion(callC) == SchedulerOpinion::MAYBE);
        REQUIRE(sch.hasHostCapacity());

        // Make calls to functions that don't breach per function limits OR host limits
        for(int i = 0; i < 3; i++) {
            sch.callFunction(callA);
            sch.callFunction(callB);
            sch.callFunction(callC);
        }

        // Check that all functions are now YES
        REQUIRE(sch.getTotalWarmFaasletCount() == 9);
        REQUIRE(sch.getLatestOpinion(callA) == SchedulerOpinion::YES);
        REQUIRE(sch.getLatestOpinion(callB) == SchedulerOpinion::YES);
        REQUIRE(sch.getLatestOpinion(callC) == SchedulerOpinion::YES);
        REQUIRE(sch.hasHostCapacity());

        // Now make a call that will breach the host limit but NOT per function limit
        sch.callFunction(callB);
        REQUIRE(sch.getLatestOpinion(callA) == SchedulerOpinion::YES);
        REQUIRE(sch.getLatestOpinion(callB) == SchedulerOpinion::NO);
        REQUIRE(sch.getLatestOpinion(callC) == SchedulerOpinion::YES);
        REQUIRE(sch.getTotalWarmFaasletCount() == 10);
        REQUIRE(!sch.hasHostCapacity());

        // Check that no sharing has been done yet
        const std::string sharingQueue = getSharingQueueNameForHost(otherHost);
        REQUIRE(redis.listLength(sharingQueue) == 0);

        // Now check that subsequent calls are shared even though they still don't
        // breach per function limits
        sch.callFunction(callA);
        sch.callFunction(callB);
        sch.callFunction(callC);

        REQUIRE(sch.getLatestOpinion(callA) == SchedulerOpinion::NO);
        REQUIRE(sch.getLatestOpinion(callB) == SchedulerOpinion::NO);
        REQUIRE(sch.getLatestOpinion(callC) == SchedulerOpinion::NO);
        REQUIRE(sch.getTotalWarmFaasletCount() == 10);
        REQUIRE(redis.listLength(sharingQueue) == 3);
        REQUIRE(!sch.hasHostCapacity());

        // Notify that a call has finished
        sch.notifyCallFinished(callA);

        // Check that, while the host still has no extra faaslet capacity
        // this function can still be executed
        REQUIRE(sch.getLatestOpinion(callA) == SchedulerOpinion::YES);
        REQUIRE(sch.getLatestOpinion(callB) == SchedulerOpinion::NO);
        REQUIRE(sch.getLatestOpinion(callC) == SchedulerOpinion::NO);
        REQUIRE(sch.getTotalWarmFaasletCount() == 10);
        REQUIRE(!sch.hasHostCapacity());

        REQUIRE(sch.getBestHostForFunction(callA) == thisHost);
        REQUIRE(sch.getBestHostForFunction(callB) == otherHost);
        REQUIRE(sch.getBestHostForFunction(callC) == otherHost);

        // Notify that a couple of faaslets have finished
        sch.notifyFaasletFinished(callA);
        sch.notifyFaasletFinished(callB);
        REQUIRE(sch.hasHostCapacity());

        // Check that now the host has capacity and we can execute another function
        // (other opinions won't have been updated yet)
        REQUIRE(sch.getLatestOpinion(callA) == SchedulerOpinion::YES);
        REQUIRE(sch.getLatestOpinion(callB) == SchedulerOpinion::YES);
        REQUIRE(sch.getLatestOpinion(callC) == SchedulerOpinion::NO);
        REQUIRE(sch.getTotalWarmFaasletCount() == 8);

        sch.callFunction(callB);
        REQUIRE(sch.getLatestOpinion(callA) == SchedulerOpinion::YES);
        REQUIRE(sch.getLatestOpinion(callB) == SchedulerOpinion::YES);
        REQUIRE(sch.getLatestOpinion(callC) == SchedulerOpinion::NO);
        REQUIRE(sch.getTotalWarmFaasletCount() == 9);
        REQUIRE(sch.hasHostCapacity());

        // Another call will breach the host limit again
        sch.callFunction(callA);
        REQUIRE(sch.getLatestOpinion(callA) == SchedulerOpinion::NO);
        REQUIRE(sch.getLatestOpinion(callB) == SchedulerOpinion::YES);
        REQUIRE(sch.getLatestOpinion(callC) == SchedulerOpinion::NO);
        REQUIRE(sch.getTotalWarmFaasletCount() == 10);
        REQUIRE(!sch.hasHostCapacity());

        // Tidy up
        conf.maxFaaslets = originalMaxFaaslet;
        conf.maxFaasletsPerFunction = originalMaxFaasletsPerFunction;
        conf.maxInFlightRatio = originalMaxInFlightRatio;
    }
}
