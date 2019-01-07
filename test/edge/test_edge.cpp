#include <catch/catch.hpp>

#include "utils.h"
#include <edge/edge.h>
#include <worker/worker.h>

using namespace Pistache;

namespace tests {
    static void setUp() {
        infra::Redis::getQueue().flushAll();

        // Create a worker pool to allow scheduling
        worker::WorkerThreadPool wp;
    }

    TEST_CASE("Test invoking a function", "[edge]") {
        cleanSystem();

        // Note - must be async to avoid needing a result
        message::Message call;
        call.set_isasync(true);
        call.set_user("demo");
        call.set_function("echo");
        call.set_inputdata("abc");

        // Get expected queue
        std::string queueName = infra::Scheduler::getFunctionQueueName(call);

        edge::FunctionEndpoint endpoint;
        endpoint.handleFunction(call);

        const message::Message actual = infra::Redis::getQueue().nextMessage(queueName);

        REQUIRE(actual.user() == "demo");
        REQUIRE(actual.function() == "echo");
        REQUIRE(actual.inputdata() == "abc");
    }

    TEST_CASE("Test invoking a non-existent function", "[worker]") {
        setUp();

        // Note - must be async to avoid needing a result
        message::Message call;
        call.set_user("foobar");
        call.set_function("baz");

        edge::FunctionEndpoint endpoint;
        std::string msg = endpoint.handleFunction(call);

        REQUIRE(msg == "foobar/baz is not a valid function");

        // Check nothing added to queue
        const std::string queueName = infra::Scheduler::getFunctionQueueName(call);
        REQUIRE(infra::Redis::getQueue().listLength(queueName) == 0);
    }
}