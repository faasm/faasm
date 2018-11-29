#include <catch/catch.hpp>
#include <infra/infra.h>
#include <util/util.h>

using namespace infra;

namespace tests {
    void checkBindMessageDispatched(Redis &cli, const message::Message &expected) {
        const message::Message actual = cli.nextMessage(infra::PREWARM_QUEUE);
        REQUIRE(actual.user() == expected.user());
        REQUIRE(actual.function() == expected.function());
        REQUIRE(actual.type() == message::Message_MessageType_BIND);
    }

    TEST_CASE("Test calling function with no workers sends bind message", "[redis]") {
        Redis cli;
        cli.flushAll();

        message::Message call;
        call.set_function("my func");
        call.set_user("some user");

        Scheduler::callFunction(call);

        checkBindMessageDispatched(cli, call);
    }

    TEST_CASE("Test calling function with existing workers does not send bind message", "[redis]") {
        Redis cli;
        cli.flushAll();

        message::Message call;
        call.set_function("my func");
        call.set_user("some user");

        std::string queueName = Scheduler::getFunctionQueueName(call);

        // Fake 2 workers initialising and binding
        Scheduler::workerInitialisedPrewarm();
        Scheduler::workerInitialisedPrewarm();

        Scheduler::workerPrewarmToBound(call);
        Scheduler::workerPrewarmToBound(call);

        // Call the function
        Scheduler::callFunction(call);

        // Check function call has been added, but no bind messages
        REQUIRE(cli.listLength(PREWARM_QUEUE) == 0);
        REQUIRE(cli.listLength(queueName) == 1);
    }

    TEST_CASE("Test calling function which breaches queue ratio sends bind message", "[redis]") {
        Redis cli;
        cli.flushAll();

        message::Message call;
        call.set_function("my func");
        call.set_user("some user");

        std::string queueName = Scheduler::getFunctionQueueName(call);

        // Fake 2 workers initialising and binding
        Scheduler::workerInitialisedPrewarm();
        Scheduler::workerInitialisedPrewarm();

        Scheduler::workerPrewarmToBound(call);
        Scheduler::workerPrewarmToBound(call);

        // Saturate up to the number of max queued calls
        util::SystemConfig conf = util::getSystemConfig();
        int nCalls = conf.max_queue_ratio * 2;
        for (int i = 0; i < nCalls; i++) {
            Scheduler::callFunction(call);
        }

        // Check no bind messages
        REQUIRE(cli.listLength(PREWARM_QUEUE) == 0);

        // Check all calls have been added to queue
        REQUIRE(cli.listLength(queueName) == nCalls);

        // Dispatch another and check that a bind message is sent
        Scheduler::callFunction(call);
        REQUIRE(cli.listLength(PREWARM_QUEUE) == 1);
        REQUIRE(cli.listLength(queueName) == nCalls + 1);

        checkBindMessageDispatched(cli, call);
    }
}
