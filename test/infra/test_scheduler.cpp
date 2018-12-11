#include <catch/catch.hpp>
#include <infra/infra.h>
#include <util/util.h>
#include "utils.h"

using namespace infra;

namespace tests {
    TEST_CASE("Test getting timeout", "[scheduler]") {
        util::SystemConfig conf = util::getSystemConfig();

        REQUIRE(Scheduler::getWorkerTimeout("blah") == conf.bound_timeout);
        REQUIRE(Scheduler::getWorkerTimeout(PREWARM_QUEUE) == conf.unbound_timeout);
        REQUIRE(Scheduler::getWorkerTimeout(COLD_QUEUE) == conf.unbound_timeout);
    }

    TEST_CASE("Test worker prewarm to bound", "[scheduler]") {
        redisQueue.flushAll();

        // Set up some fake prewarms
        Scheduler::prewarmWorker();
        Scheduler::prewarmWorker();
        REQUIRE(Scheduler::getPrewarmCount() == 2);

        // Set up call
        message::Message call;
        call.set_user("userA");
        call.set_function("funcA");

        // Check the right queue name is returned
        std::string actualA = Scheduler::workerPrewarmToBound(call);
        REQUIRE(actualA == Scheduler::getFunctionQueueName(call));

        // Check that the prewarm count is *not* updated and other messages not sent.
        // This is done in the original bind call
        REQUIRE(Scheduler::getPrewarmCount() == 2);
        REQUIRE(redisQueue.listLength(PREWARM_QUEUE) == 0);
        REQUIRE(redisQueue.listLength(COLD_QUEUE) == 0);
    }

    TEST_CASE("Test worker finished prewarm", "[scheduler]") {
        redisQueue.flushAll();

        Scheduler::prewarmWorker();
        REQUIRE(Scheduler::getPrewarmCount() == 1);

        Scheduler::workerFinished(PREWARM_QUEUE);

        REQUIRE(Scheduler::getPrewarmCount() == 0);
    }

    TEST_CASE("Test worker finished cold", "[scheduler]") {
        redisQueue.flushAll();

        // Fill up all prewarm
        util::SystemConfig conf = util::getSystemConfig();
        for (int i = 0; i < conf.prewarm_target; i++) {
            Scheduler::prewarmWorker();
        }

        // Add a cold init
        Scheduler::prewarmWorker();
        REQUIRE(Scheduler::getColdCount() == 1);

        Scheduler::workerFinished(COLD_QUEUE);

        REQUIRE(Scheduler::getColdCount() == 0);
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

        // Set up a dummy prewarm
        Scheduler::prewarmWorker();
        REQUIRE(Scheduler::getFunctionCount(call) == 0);
        REQUIRE(Scheduler::getPrewarmCount() == 1);

        // Call the function
        Scheduler::callFunction(call);

        // Check function count has increased and bind message sent
        REQUIRE(Scheduler::getFunctionCount(call) == 1);
        REQUIRE(redisQueue.listLength(PREWARM_QUEUE) == 1);

        // Check prewarm count has been decreased and prewarm message sent
        REQUIRE(Scheduler::getPrewarmCount() == 0);
        REQUIRE(redisQueue.listLength(COLD_QUEUE) == 1);
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

        // Call each function
        Scheduler::callFunction(callA);
        Scheduler::callFunction(callB);

        // Check that counters are updated (should be done before bind is dispatched)
        REQUIRE(Scheduler::getFunctionCount(callA) == 1);
        REQUIRE(Scheduler::getFunctionCount(callB) == 1);

        // Check that bind messages have been sent
        const message::Message bindA = redisQueue.nextMessage(PREWARM_QUEUE);
        const message::Message bindB = redisQueue.nextMessage(PREWARM_QUEUE);

        REQUIRE(bindA.user() == callA.user());
        REQUIRE(bindA.function() == callA.function());
        REQUIRE(bindA.type() == message::Message_MessageType_BIND);

        REQUIRE(bindB.user() == callB.user());
        REQUIRE(bindB.function() == callB.function());
        REQUIRE(bindB.type() == message::Message_MessageType_BIND);

        // Check that prewarm messages have been sent
        const message::Message prewarmA = redisQueue.nextMessage(COLD_QUEUE);
        const message::Message prewarmB = redisQueue.nextMessage(COLD_QUEUE);

        REQUIRE(prewarmA.user().empty());
        REQUIRE(prewarmA.function().empty());
        REQUIRE(prewarmA.type() == message::Message_MessageType_PREWARM);

        REQUIRE(prewarmB.user().empty());
        REQUIRE(prewarmB.function().empty());
        REQUIRE(prewarmB.type() == message::Message_MessageType_PREWARM);
    }

    TEST_CASE("Test calling function with existing workers does not send bind message", "[scheduler]") {
        redisQueue.flushAll();

        message::Message call;
        call.set_function("my func");
        call.set_user("some user");

        std::string queueName = Scheduler::getFunctionQueueName(call);

        // Fake 2 workers initialising to prewarm
        Scheduler::prewarmWorker();
        Scheduler::prewarmWorker();

        // Fake calling the function first, should cause a bind message and set up the function queue
        Scheduler::callFunction(call);
        REQUIRE(redisQueue.listLength(PREWARM_QUEUE) == 1);
        REQUIRE(redisQueue.listLength(queueName) == 1);

        // Call the function again
        Scheduler::callFunction(call);

        // Check function call has been added, but no new bind messages
        REQUIRE(redisQueue.listLength(PREWARM_QUEUE) == 1);
        REQUIRE(redisQueue.listLength(queueName) == 2);
    }

    TEST_CASE("Test calling function which breaches queue ratio sends bind message", "[scheduler]") {
        redisQueue.flushAll();

        message::Message call;
        call.set_function("my func");
        call.set_user("some user");

        std::string queueName = Scheduler::getFunctionQueueName(call);

        // Saturate up to the number of max queued calls
        util::SystemConfig conf = util::getSystemConfig();
        int nCalls = conf.max_queue_ratio - 1;
        for (int i = 0; i < nCalls; i++) {
            Scheduler::callFunction(call);

            // Check only one bind message is ever sent
            REQUIRE(redisQueue.listLength(PREWARM_QUEUE) == 1);

            // Check call queued
            REQUIRE(redisQueue.listLength(queueName) == i + 1);
        }

        // Dispatch another and check that a bind message is sent
        Scheduler::callFunction(call);
        REQUIRE(redisQueue.listLength(PREWARM_QUEUE) == 2);
        REQUIRE(redisQueue.listLength(queueName) == nCalls + 1);
    }

    TEST_CASE("Test function which breaches queue ratio but has no capacity does not scale up", "[scheduler]") {
        redisQueue.flushAll();

        message::Message call;
        call.set_user("userA");
        call.set_function("funcA");

        std::string queueName = Scheduler::getFunctionQueueName(call);

        // Make calls up to the limit
        util::SystemConfig conf = util::getSystemConfig();
        int nCalls = conf.max_workers_per_function * conf.max_queue_ratio;
        for (int i = 0; i < nCalls; i++) {
            Scheduler::callFunction(call);
        }

        // Check number of bind messages
        REQUIRE(redisQueue.listLength(PREWARM_QUEUE) == conf.max_workers_per_function);

        // Check count assigned to function
        REQUIRE(Scheduler::getFunctionCount(call) == conf.max_workers_per_function);

        // Check calls have been queued
        REQUIRE(redisQueue.listLength(Scheduler::getFunctionQueueName(call)) == nCalls);

        // Now call the function a few more times and check calls are queued but no new bind messages/ workers added
        Scheduler::callFunction(call);
        Scheduler::callFunction(call);
        Scheduler::callFunction(call);

        REQUIRE(redisQueue.listLength(PREWARM_QUEUE) == conf.max_workers_per_function);
        REQUIRE(Scheduler::getFunctionCount(call) == conf.max_workers_per_function);
        REQUIRE(redisQueue.listLength(Scheduler::getFunctionQueueName(call)) == nCalls + 3);
    }

    TEST_CASE("Test is need to prewarm", "[scheduler]") {
        redisQueue.flushAll();

        // Check should prewarm up to limit
        const util::SystemConfig conf = util::getSystemConfig();
        for (int i = 0; i < conf.prewarm_target; i++) {
            REQUIRE(Scheduler::prewarmWorker());
            REQUIRE(Scheduler::getPrewarmCount() == i + 1);
            REQUIRE(Scheduler::getColdCount() == 0);
        }

        // Should be false once we've hit the target
        for (int i = 0; i < 5; i++) {
            REQUIRE(!Scheduler::prewarmWorker());
            REQUIRE(Scheduler::getPrewarmCount() == conf.prewarm_target);
            REQUIRE(Scheduler::getColdCount() == i + 1);
        }
    }
}
