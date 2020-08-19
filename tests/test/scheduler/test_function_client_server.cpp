#include <catch/catch.hpp>

#include "utils.h"

#include <scheduler/FunctionCallClient.h>
#include <scheduler/FunctionCallServer.h>
#include <util/config.h>

using namespace scheduler;

namespace tests {
    TEST_CASE("Test sending function call", "[state]") {
        // Start the server        
        ServerContext serverContext;
        FunctionCallServer server;
        server.start();
        usleep(1000 * 100);

        // Create a message
        message::Message msg;
        msg.set_user("demo");
        msg.set_function("echo");
        msg.set_inputdata("foobarbaz");

        // Get the queue for the function
        Scheduler &sch = scheduler::getScheduler();
        std::shared_ptr<InMemoryMessageQueue> funcQueue = sch.getFunctionQueue(msg);

        // Check queue is empty
        REQUIRE(funcQueue->size() == 0);
        
        // Send the message to the server
        FunctionCallClient client(LOCALHOST);
        client.shareFunctionCall(msg);

        // Check the message is on the queue
        REQUIRE(funcQueue->size() == 1);
        message::Message actual = funcQueue->dequeue();
        REQUIRE(actual.user() == msg.user());
        REQUIRE(actual.function() == msg.function());
        REQUIRE(actual.inputdata() == msg.inputdata());

        // Stop the server
        server.stop();
    }
}