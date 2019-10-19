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

        std::string nodeId = util::getNodeId();
        Redis &redis = Redis::getQueue();

        Scheduler s;

        funcSet = s.getFunctionWarmSetName(call);

        // Check initial set-up
        REQUIRE(redis.sismember(GLOBAL_NODE_SET, nodeId));
        REQUIRE(!redis.sismember(funcSet, nodeId));

        // Call the function and check it's added to the function's warm set
        s.callFunction(call);

        REQUIRE(redis.sismember(GLOBAL_NODE_SET, nodeId));
        REQUIRE(redis.sismember(funcSet, nodeId));

        // Run clear-up
        s.clear();

        // After clear-up has run this node should no longer be part of either set
        REQUIRE(!redis.sismember(GLOBAL_NODE_SET, nodeId));
        REQUIRE(!redis.sismember(funcSet, nodeId));
    }

    TEST_CASE("Test scheduler operations", "[scheduler]") {
        cleanSystem();

        Scheduler &sch = scheduler::getScheduler();
        Redis &redis = Redis::getQueue();

        std::string nodeId = util::getNodeId();
        std::string otherNodeA = "node A";
        SharingMessageBus &sharingBus = SharingMessageBus::getInstance();

        message::Message call = util::messageFactory("user a", "function a");
        message::Message chainedCall = util::messageFactory("user a", "function a");
        chainedCall.set_idx(3);

        util::SystemConfig &conf = util::getSystemConfig();
        int originalMaxQueueRatio = conf.maxQueueRatio;
        conf.maxQueueRatio = 8;

        SECTION("Test worker finishing and node removal") {
            // Sanity check to see that the node is in the global set but not the set for the function
            std::string funcSet = sch.getFunctionWarmSetName(call);
            REQUIRE(redis.sismember(GLOBAL_NODE_SET, nodeId));
            REQUIRE(!redis.sismember(funcSet, nodeId));

            // Call the function enough to get multiple workers set up
            int requiredCalls = (conf.maxQueueRatio * 2) - 1;
            for (int i = 0; i < requiredCalls; i++) {
                sch.callFunction(call);
            }

            // Check node is now part of function's set
            REQUIRE(redis.sismember(funcSet, nodeId));
            REQUIRE(sch.getFunctionQueueLength(call) == requiredCalls);
            REQUIRE(sch.getBindQueue()->size() == 2);

            // Notify that a worker has finished, count decremented by one and worker is still member of function set
            sch.stopListeningToQueue(call);
            REQUIRE(redis.sismember(funcSet, nodeId));
            REQUIRE(sch.getFunctionQueueLength(call) == requiredCalls);
            REQUIRE(sch.getFunctionThreadCount(call) == 1);

            // Notify that another worker has finished, check count is decremented and node removed from function set
            // (but still in global set)
            sch.stopListeningToQueue(call);
            REQUIRE(redis.sismember(GLOBAL_NODE_SET, nodeId));
            REQUIRE(!redis.sismember(funcSet, nodeId));
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

            // Add a worker node to the global set
            redis.sadd(GLOBAL_NODE_SET, "foo");

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

        SECTION("Test calling function with existing workers does not send bind message") {
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

        SECTION("Test calling function which breaches queue ratio sends bind message") {
            // Saturate up to the number of max queued calls
            auto bindQueue = sch.getBindQueue();
            int nCalls = conf.maxQueueRatio - 1;
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

        SECTION("Test chained calls use lower queue ratio") {
            // Add a few normal calls below the normal queue ratio but
            // above the chained queue ratio (i.e. 1)
            auto bindQueue = sch.getBindQueue();
            int nCalls = conf.maxQueueRatio - 3;
            for (int i = 0; i < nCalls; i++) {
                sch.callFunction(call);
            }

            // Dispatch another and check that a bind message is still not sent
            sch.callFunction(call);
            REQUIRE(bindQueue->size() == 1);
            REQUIRE(sch.getFunctionQueueLength(call) == nCalls + 1);

            // Now dispatch a chained call and check that it causes another bind,
            // and is put on the same queue as the normal messages
            sch.callFunction(chainedCall);
            REQUIRE(bindQueue->size() == 2);
            REQUIRE(sch.getFunctionQueueLength(call) == nCalls + 2);

            redis.flushAll();
        }

        SECTION("Node choice checks") {
            redis.sadd(GLOBAL_NODE_SET, otherNodeA);

            SECTION("Test function which breaches queue ratio but has no capacity fails over") {
                // Make calls up to the limit
                int nCalls = conf.maxWorkersPerFunction * conf.maxQueueRatio;
                for (int i = 0; i < nCalls; i++) {
                    sch.callFunction(call);
                }

                // Check local workers requested
                auto bindQueue = sch.getBindQueue();
                REQUIRE(bindQueue->size() == conf.maxWorkersPerFunction);
                REQUIRE(sch.getFunctionThreadCount(call) == conf.maxWorkersPerFunction);

                // Check calls have been queued
                REQUIRE(sch.getFunctionQueueLength(call) == nCalls);

                // Check "best node" is now different
                REQUIRE(sch.getBestNodeForFunction(call) != nodeId);

                // Call more and check calls are shared elsewhere
                sch.callFunction(call);
                sch.callFunction(call);

                message::Message actualA = sharingBus.nextMessageForNode(otherNodeA);
                message::Message actualB = sharingBus.nextMessageForNode(otherNodeA);

                REQUIRE(actualA.function() == call.function());
                REQUIRE(actualA.user() == call.user());

                REQUIRE(actualB.function() == call.function());
                REQUIRE(actualB.user() == call.user());

                // Check not added to local queues
                REQUIRE(bindQueue->size() == conf.maxWorkersPerFunction);
                REQUIRE(sch.getFunctionThreadCount(call) == conf.maxWorkersPerFunction);
                REQUIRE(sch.getFunctionQueueLength(call) == nCalls);

                redis.flushAll();
            }

            SECTION("Test chained function fails over at lower limit") {
                // Make a few calls but still below the normal limit
                int nCalls = (conf.maxWorkersPerFunction * conf.maxQueueRatio) - 2;
                for (int i = 0; i < nCalls; i++) {
                    sch.callFunction(call);
                }

                // Check "best node" is still this node
                REQUIRE(sch.getBestNodeForFunction(call) == nodeId);

                // Check "best node" for a chained call is the other node
                REQUIRE(sch.getBestNodeForFunction(chainedCall) != nodeId);
            }

            SECTION("Test scheduler requests scale-out when no options") {
                // Clear out worker set and just add this node
                redis.flushAll();
                redis.sadd(GLOBAL_NODE_SET, nodeId);

                GlobalMessageBus &globalBus = getGlobalMessageBus();
                globalBus.clear();

                REQUIRE(sch.getGlobalSetSize() == 1);
                REQUIRE(globalBus.getScaleoutRequestCount() == 0);

                // Saturate this node
                int nCalls = conf.maxWorkersPerFunction * conf.maxQueueRatio;
                for (int i = 0; i < nCalls; i++) {
                    sch.callFunction(call);
                }

                // Make another request
                sch.callFunction(call);

                // Check scale-out request has been made
                REQUIRE(globalBus.getScaleoutRequestCount() == 1);
            }

            SECTION("Test scheduler adds node ID to global set when starting up") {
                REQUIRE(!nodeId.empty());
                REQUIRE(redis.sismember(GLOBAL_NODE_SET, nodeId));
            }

            SECTION("Test current node chosen when no warm alternatives") {
                REQUIRE(sch.getBestNodeForFunction(call) == nodeId);
            }

            SECTION("Test other warm option chosen when available") {
                // Add another node to the warm set for the given function
                const std::string warmSet = sch.getFunctionWarmSetName(call);
                redis.sadd(warmSet, otherNodeA);

                REQUIRE(sch.getBestNodeForFunction(call) == otherNodeA);
            }

            SECTION("Test other warm option *not* chosen when in no scheduler mode") {
                conf.noScheduler = 1;

                // Add another node to the warm set for the given function
                const std::string warmSet = sch.getFunctionWarmSetName(call);
                redis.sadd(warmSet, otherNodeA);

                // Check it's ignored
                REQUIRE(sch.getBestNodeForFunction(call) == nodeId);

                conf.noScheduler = 0;
            }

            SECTION("Test current node chosen when already warm even if alternatives") {
                // Ensure a warm worker exists on this node
                sch.callFunction(call);
                REQUIRE(sch.getFunctionThreadCount(call) == 1);

                // Check this node is in the warm set
                const std::string warmSet = sch.getFunctionWarmSetName(call);
                REQUIRE(redis.sismember(warmSet, nodeId));

                // Add another node to the warm set
                redis.sadd(warmSet, otherNodeA);

                // Run a few times to make sure
                REQUIRE(sch.getBestNodeForFunction(call) == nodeId);
                REQUIRE(sch.getBestNodeForFunction(call) == nodeId);
                REQUIRE(sch.getBestNodeForFunction(call) == nodeId);
                REQUIRE(sch.getBestNodeForFunction(call) == nodeId);
            }
        }

        conf.maxQueueRatio = originalMaxQueueRatio;
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
            redis.sadd(GLOBAL_NODE_SET, "foo");
            redis.sadd(GLOBAL_NODE_SET, "bar");

            // Request a scale-out (should always dispatch only one request at a time regardless of the target)
            sch.scaleOut(2);

            REQUIRE(globalBus.getScaleoutRequestCount() == 0);
        }

        SECTION("Check scale out ignored when at max") {
            // Add workers up to the limit
            for (int i = 0; i < conf.maxNodes; i++) {
                std::string workerName = "foo" + std::to_string(i);
                redis.sadd(GLOBAL_NODE_SET, workerName);
            }

            // Request a scale-out over the max
            sch.scaleOut(conf.maxNodes + 1);

            REQUIRE(globalBus.getScaleoutRequestCount() == 0);
        }
    }

    TEST_CASE("Test message recording of scheduling decisions", "[scheduler]") {
        cleanSystem();
        Scheduler &sch = scheduler::getScheduler();
        SharingMessageBus &sharingBus = SharingMessageBus::getInstance();

        std::string thisNodeId = util::getNodeId();
        std::string otherNodeA = "node A";

        util::SystemConfig &conf = util::getSystemConfig();
        int maxQueueRatio = conf.maxQueueRatio;
        int maxWorkers = conf.maxWorkersPerFunction;

        // Add calls to saturate the first node
        int requiredCalls = maxQueueRatio * maxWorkers;
        for (int i = 0; i < requiredCalls; i++) {
            message::Message msgA = util::messageFactory("demo", "chain_simple");
            sch.callFunction(msgA);

            // Check scheduling info
            REQUIRE(msgA.schedulednode() == thisNodeId);
            REQUIRE(msgA.hops() == 0);
            REQUIRE(msgA.executednode().empty());
        }

        // Now add the other node to the warm set and make
        // sure calls are sent there
        message::Message msgB = util::messageFactory("demo", "chain_simple");
        const std::string warmSet = sch.getFunctionWarmSetName(msgB);
        Redis &redis = redis::Redis::getQueue();
        redis.sadd(warmSet, otherNodeA);

        for (int i = 0; i < 3; i++) {
            message::Message msgC = util::messageFactory("demo", "chain_simple");
            sch.callFunction(msgC);

            // Check scheduling info
            REQUIRE(msgC.schedulednode() == otherNodeA);
            REQUIRE(msgC.hops() == 1);
            REQUIRE(msgC.executednode().empty());

            // Check actual message bus
            message::Message actualShare = sharingBus.nextMessageForNode(otherNodeA);
        }
    }

    TEST_CASE("Test multiple hops", "[scheduler]") {
        cleanSystem();
        Scheduler &sch = scheduler::getScheduler();
        SharingMessageBus &sharingBus = SharingMessageBus::getInstance();

        std::string thisNodeId = util::getNodeId();
        std::string otherNodeA = "node A";

        message::Message msg = util::messageFactory("demo", "chain_simple");

        // Add calls to saturate the first node
        util::SystemConfig &conf = util::getSystemConfig();
        int requiredCalls = conf.maxQueueRatio * conf.maxWorkersPerFunction;
        for (int i = 0; i < requiredCalls; i++) {
            sch.callFunction(msg);
        }

        // Add other node to warm set
        const std::string warmSet = sch.getFunctionWarmSetName(msg);
        Redis &redis = redis::Redis::getQueue();
        redis.sadd(warmSet, otherNodeA);

        // Now create a message that's already got a scheduled node and hops
        message::Message msgWithHops = util::messageFactory("demo", "chain_simple");
        msgWithHops.set_schedulednode("Some other node");
        msgWithHops.set_hops(5);

        // Schedule the call
        sch.callFunction(msgWithHops);

        // Check host not changes and hops increased
        REQUIRE(msgWithHops.schedulednode() == "Some other node");
        REQUIRE(msgWithHops.hops() == 6);
    }
}
