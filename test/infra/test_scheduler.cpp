#include <catch/catch.hpp>
#include <infra/infra.h>
#include <util/util.h>

using namespace infra;

namespace tests {
    TEST_CASE("Test getting prewarm/ cold count", "[scheduler]") {
        REQUIRE(Scheduler::getPrewarmCount() == 0);
        REQUIRE(Scheduler::getColdCount() == 0);

        Scheduler::workerInitialisedPrewarm();
        Scheduler::workerInitialisedPrewarm();

        REQUIRE(Scheduler::getPrewarmCount() == 2);
        REQUIRE(Scheduler::getColdCount() == 0);

        Scheduler::workerInitialisedCold();
        Scheduler::workerInitialisedCold();
        Scheduler::workerInitialisedCold();

        REQUIRE(Scheduler::getPrewarmCount() == 2);
        REQUIRE(Scheduler::getColdCount() == 3);
    }

    TEST_CASE("Test getting timeout", "[scheduler]") {
        util::SystemConfig conf = util::getSystemConfig();

        REQUIRE(Scheduler::getWorkerTimeout("blah") == conf.bound_timeout);
        REQUIRE(Scheduler::getWorkerTimeout(PREWARM_QUEUE) == conf.unbound_timeout);
        REQUIRE(Scheduler::getWorkerTimeout(COLD_QUEUE) == conf.unbound_timeout);
    }

    TEST_CASE("Test calling function with no workers sends bind message", "[scheduler]") {
        Redis cli;
        cli.flushAll();

        message::Message call;
        call.set_function("my func");
        call.set_user("some user");

        Scheduler::callFunction(call);

        REQUIRE(cli.listLength(PREWARM_QUEUE) == 1);
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
}
