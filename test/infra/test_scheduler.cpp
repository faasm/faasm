#include <catch/catch.hpp>
#include <infra/infra.h>
#include <util/util.h>

using namespace infra;

namespace tests {
    TEST_CASE("Test getting prewarm/ cold count", "[scheduler]") {
        Redis cli;
        cli.flushAll();

        REQUIRE(Scheduler::getPrewarmCount() == 0);
        REQUIRE(Scheduler::getColdCount() == 0);

        REQUIRE(Scheduler::workerInitialisedPrewarm() == PREWARM_QUEUE);
        REQUIRE(Scheduler::workerInitialisedPrewarm() == PREWARM_QUEUE);

        REQUIRE(Scheduler::getPrewarmCount() == 2);
        REQUIRE(Scheduler::getColdCount() == 0);

        REQUIRE(Scheduler::workerInitialisedCold() == COLD_QUEUE);
        REQUIRE(Scheduler::workerInitialisedCold() == COLD_QUEUE);
        REQUIRE(Scheduler::workerInitialisedCold() == COLD_QUEUE);

        REQUIRE(Scheduler::getPrewarmCount() == 2);
        REQUIRE(Scheduler::getColdCount() == 3);
    }

    TEST_CASE("Test getting timeout", "[scheduler]") {
        util::SystemConfig conf = util::getSystemConfig();

        REQUIRE(Scheduler::getWorkerTimeout("blah") == conf.bound_timeout);
        REQUIRE(Scheduler::getWorkerTimeout(PREWARM_QUEUE) == conf.unbound_timeout);
        REQUIRE(Scheduler::getWorkerTimeout(COLD_QUEUE) == conf.unbound_timeout);
    }

    TEST_CASE("Test need to prewarm", "[scheduler]") {
        Redis cli;
        cli.flushAll();

        REQUIRE(Scheduler::isNeedToPrewarm());

        // Add target number of prewarm workers, check at before each that we still need to add more
        util::SystemConfig conf = util::getSystemConfig();
        for (int i = 0; i < conf.prewarm_target; i++) {
            REQUIRE(Scheduler::isNeedToPrewarm());
            Scheduler::workerInitialisedPrewarm();
        }

        // Check no longer need to prewarm
        REQUIRE(!Scheduler::isNeedToPrewarm());
    }

    TEST_CASE("Test worker cold to prewarm", "[scheduler]") {
        Redis cli;
        cli.flushAll();

        // Set up some fake cold
        Scheduler::workerInitialisedCold();
        Scheduler::workerInitialisedCold();
        Scheduler::workerInitialisedCold();
        REQUIRE(Scheduler::getPrewarmCount() == 0);
        REQUIRE(Scheduler::getColdCount() == 3);

        Scheduler::workerColdToPrewarm();
        REQUIRE(Scheduler::getPrewarmCount() == 1);
        REQUIRE(Scheduler::getColdCount() == 2);

        Scheduler::workerColdToPrewarm();
        REQUIRE(Scheduler::getPrewarmCount() == 2);
        REQUIRE(Scheduler::getColdCount() == 1);
    }

    TEST_CASE("Test worker prewarm to bound", "[scheduler]") {
        Redis cli;
        cli.flushAll();

        // Set up some fake prewarms
        Scheduler::workerInitialisedPrewarm();
        Scheduler::workerInitialisedPrewarm();
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
        REQUIRE(cli.listLength(PREWARM_QUEUE) == 0);
        REQUIRE(cli.listLength(COLD_QUEUE) == 0);
    }

    TEST_CASE("Test worker finished prewarm", "[scheduler]") {
        Redis cli;
        cli.flushAll();

        Scheduler::workerInitialisedPrewarm();
        REQUIRE(Scheduler::getPrewarmCount() == 1);

        Scheduler::workerFinished(PREWARM_QUEUE);

        REQUIRE(Scheduler::getPrewarmCount() == 0);
    }

    TEST_CASE("Test worker finished cold", "[scheduler]") {
        Redis cli;
        cli.flushAll();

        Scheduler::workerInitialisedCold();
        REQUIRE(Scheduler::getColdCount() == 1);

        Scheduler::workerFinished(COLD_QUEUE);

        REQUIRE(Scheduler::getColdCount() == 0);
    }

    TEST_CASE("Test worker finished bound", "[scheduler]") {
        Redis cli;
        cli.flushAll();

        message::Message call;
        call.set_user("userA");
        call.set_function("funcA");

        // Set up a worker on the given function
        Scheduler::workerInitialisedPrewarm();
        Scheduler::sendBindMessage(call);
        REQUIRE(Scheduler::getFunctionCount(call) == 1);

        // Mark it as finished
        Scheduler::workerFinished(Scheduler::getFunctionQueueName(call));
        REQUIRE(Scheduler::getFunctionCount(call) == 0);
    }

    TEST_CASE("Test calling function with no workers sends bind message", "[scheduler]") {
        Redis cli;
        cli.flushAll();

        message::Message call;
        call.set_function("my func");
        call.set_user("some user");

        // Set up a dummy prewarm
        Scheduler::workerInitialisedPrewarm();
        REQUIRE(Scheduler::getFunctionCount(call) == 0);
        REQUIRE(Scheduler::getPrewarmCount() == 1);

        // Call the function
        Scheduler::callFunction(call);

        // Check function count has increased and bind message sent
        REQUIRE(Scheduler::getFunctionCount(call) == 1);
        REQUIRE(cli.listLength(PREWARM_QUEUE) == 1);

        // Check prewarm count has been decreased and prewarm message sent
        REQUIRE(Scheduler::getPrewarmCount() == 0);
        REQUIRE(cli.listLength(COLD_QUEUE) == 1);
    }

    TEST_CASE("Test sending bind messages", "[scheduler]") {
        Redis cli;
        cli.flushAll();

        // Fake a couple of prewarms
        Scheduler::workerInitialisedPrewarm();
        Scheduler::workerInitialisedPrewarm();

        // Bind to a couple of functions
        message::Message callA;
        callA.set_user("userA");
        callA.set_function("funcA");

        message::Message callB;
        callB.set_user("userB");
        callB.set_function("funcB");

        Scheduler::sendBindMessage(callA);
        Scheduler::sendBindMessage(callB);

        // Check that counters updated
        REQUIRE(Scheduler::getFunctionCount(callA) == 1);
        REQUIRE(Scheduler::getFunctionCount(callB) == 1);
        REQUIRE(Scheduler::getPrewarmCount() == 0);

        // Check that bind messages have been sent
        const message::Message bindA = cli.nextMessage(PREWARM_QUEUE);
        const message::Message bindB = cli.nextMessage(PREWARM_QUEUE);

        REQUIRE(bindA.user() == callA.user());
        REQUIRE(bindA.function() == callA.function());
        REQUIRE(bindA.type() == message::Message_MessageType_BIND);

        REQUIRE(bindB.user() == callB.user());
        REQUIRE(bindB.function() == callB.function());
        REQUIRE(bindB.type() == message::Message_MessageType_BIND);

        // Check that prewarm messages have been sent
        const message::Message prewarmA = cli.nextMessage(COLD_QUEUE);
        const message::Message prewarmB = cli.nextMessage(COLD_QUEUE);

        REQUIRE(prewarmA.user().empty());
        REQUIRE(prewarmA.function().empty());
        REQUIRE(prewarmA.type() == message::Message_MessageType_PREWARM);

        REQUIRE(prewarmB.user().empty());
        REQUIRE(prewarmB.function().empty());
        REQUIRE(prewarmB.type() == message::Message_MessageType_PREWARM);
    }

    TEST_CASE("Test calling function with existing workers does not send bind message", "[scheduler]") {
        Redis cli;
        cli.flushAll();

        message::Message call;
        call.set_function("my func");
        call.set_user("some user");

        std::string queueName = Scheduler::getFunctionQueueName(call);

        // Fake 2 workers initialising
        Scheduler::workerInitialisedPrewarm();
        Scheduler::workerInitialisedPrewarm();

        // Fake asking two workers to bind
        Scheduler::sendBindMessage(call);
        Scheduler::sendBindMessage(call);
        REQUIRE(cli.listLength(PREWARM_QUEUE) == 2);
        REQUIRE(cli.listLength(queueName) == 0);

        // Call the function
        Scheduler::callFunction(call);

        // Check function call has been added, but no new bind messages
        REQUIRE(cli.listLength(PREWARM_QUEUE) == 2);
        REQUIRE(cli.listLength(queueName) == 1);
    }

    TEST_CASE("Test calling function which breaches queue ratio sends bind message", "[scheduler]") {
        Redis cli;
        cli.flushAll();

        message::Message call;
        call.set_function("my func");
        call.set_user("some user");

        std::string queueName = Scheduler::getFunctionQueueName(call);

        // Fake 2 workers initialising
        Scheduler::workerInitialisedPrewarm();
        Scheduler::workerInitialisedPrewarm();

        // Fake requesting 2 binds
        Scheduler::sendBindMessage(call);
        Scheduler::sendBindMessage(call);
        REQUIRE(cli.listLength(PREWARM_QUEUE) == 2);

        // Saturate up to the number of max queued calls
        util::SystemConfig conf = util::getSystemConfig();
        int nCalls = conf.max_queue_ratio * 2;
        for (int i = 0; i < nCalls; i++) {
            Scheduler::callFunction(call);
        }

        // Check no new bind messages
        REQUIRE(cli.listLength(PREWARM_QUEUE) == 2);

        // Check all calls have been added to queue
        REQUIRE(cli.listLength(queueName) == nCalls);

        // Dispatch another and check that a bind message is sent
        Scheduler::callFunction(call);
        REQUIRE(cli.listLength(PREWARM_QUEUE) == 3);
        REQUIRE(cli.listLength(queueName) == nCalls + 1);
    }

    TEST_CASE("Test function which breaches queue ratio but has no capacity does not scale up", "[scheduler]") {
        Redis cli;
        cli.flushAll();

        message::Message call;
        call.set_user("userA");
        call.set_function("funcA");

        std::string queueName = Scheduler::getFunctionQueueName(call);

        // Set up as though the max workers are already listening
        util::SystemConfig conf = util::getSystemConfig();
        for(int i = 0; i < conf.max_workers_per_function; i++) {
            Scheduler::workerInitialisedPrewarm();
            Scheduler::sendBindMessage(call);
        }

        // Call the function the max numbet of times
        int nCalls = conf.max_workers_per_function * conf.max_queue_ratio;
        for(int i = 0; i < nCalls; i++) {
            Scheduler::callFunction(call);
        }

        // Check set-up
        REQUIRE(cli.listLength(PREWARM_QUEUE) == conf.max_workers_per_function);
        REQUIRE(Scheduler::getFunctionCount(call) == conf.max_workers_per_function);
        REQUIRE(cli.listLength(Scheduler::getFunctionQueueName(call)) == nCalls);

        // Now call the function a few more times and check calls are queued but no new bind messages sent
        Scheduler::callFunction(call);
        Scheduler::callFunction(call);
        Scheduler::callFunction(call);

        REQUIRE(cli.listLength(PREWARM_QUEUE) == conf.max_workers_per_function);
        REQUIRE(Scheduler::getFunctionCount(call) == conf.max_workers_per_function);
        REQUIRE(cli.listLength(Scheduler::getFunctionQueueName(call)) == nCalls + 3);
    }
}
