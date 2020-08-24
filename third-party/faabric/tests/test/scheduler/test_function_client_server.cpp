#include <catch/catch.hpp>

#include "utils.h"

#include <scheduler/FunctionCallClient.h>
#include <scheduler/FunctionCallServer.h>
#include <util/config.h>
#include <scheduler/MpiWorld.h>
#include <scheduler/MpiWorldRegistry.h>

using namespace scheduler;

namespace tests {
    TEST_CASE("Test sending function call", "[scheduler]") {
        // Start the server        
        ServerContext serverContext;
        FunctionCallServer server;
        server.start();
        usleep(1000 * 100);

        // Create a message
        faabric::Message msg;
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
        faabric::Message actual = funcQueue->dequeue();
        REQUIRE(actual.user() == msg.user());
        REQUIRE(actual.function() == msg.function());
        REQUIRE(actual.inputdata() == msg.inputdata());

        // Stop the server
        server.stop();
    }
    
    TEST_CASE("Test sending MPI message", "[scheduler]") {
        // Start the server        
        ServerContext serverContext;
        FunctionCallServer server;
        server.start();
        usleep(1000 * 100);

        // Create an MPI world on this host and one on a "remote" host
        std::string otherHost = "192.168.9.2";

        const char *user = "mpi";
        const char *func = "hellompi";
        const faabric::Message &msg = util::messageFactory(user, func);
        int worldId = 123;
        int worldSize = 2;

        scheduler::MpiWorldRegistry &registry = getMpiWorldRegistry();
        scheduler::MpiWorld &localWorld = registry.createWorld(msg, worldId);
        localWorld.overrideHost(LOCALHOST);
        localWorld.create(msg, worldId, worldSize);

        scheduler::MpiWorld remoteWorld;
        remoteWorld.overrideHost(otherHost);
        remoteWorld.initialiseFromState(msg, worldId);
        
        // Register a rank on each
        int rankLocal = 0;
        int rankRemote = 1;
        localWorld.registerRank(rankLocal);
        remoteWorld.registerRank(rankRemote);
        
        // Create a message
        faabric::MPIMessage mpiMsg;
        mpiMsg.set_worldid(worldId);
        mpiMsg.set_sender(rankRemote);
        mpiMsg.set_destination(rankLocal);
        
        // Send the message
        FunctionCallClient cli(LOCALHOST);
        cli.sendMPIMessage(mpiMsg);
        
        // Make sure the message has been put on the right queue locally
        std::shared_ptr<InMemoryMpiQueue> queue = localWorld.getLocalQueue(rankRemote, rankLocal);
        REQUIRE(queue->size() == 1);
        const faabric::MPIMessage &actualMessage = queue->dequeue();

        REQUIRE(actualMessage.worldid() == worldId);
        REQUIRE(actualMessage.sender() == rankRemote);

        // Stop the server
        server.stop();
    }
}