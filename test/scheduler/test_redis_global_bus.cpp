#include <catch/catch.hpp>

#include "utils.h"

#include <scheduler/Scheduler.h>

#include <util/queue.h>

using namespace scheduler;

namespace tests {

    TEST_CASE("Global message queue tests", "[scheduler]") {
        cleanSystem();

        redis::Redis &redis = redis::Redis::getQueue();
        GlobalMessageBus &bus = getGlobalMessageBus();
        util::SystemConfig &conf = util::getSystemConfig();

        // Request function
        std::string funcName = "my func";
        std::string userName = "some user";
        std::string inputData = "blahblah";
        message::Message call = util::messageFactory(userName, funcName);
        call.set_inputdata(inputData);

        std::string originalSerialisation = conf.serialisation;

        SECTION("Round trip checks") {
            SECTION("True boolean over JSON") {
                call.set_isasync(true);
                call.set_ispython(true);
                call.set_istypescript(true);
                conf.serialisation = "json";
            }

            SECTION("False boolean over JSON") {
                call.set_isasync(false);
                call.set_ispython(false);
                call.set_istypescript(false);
                conf.serialisation = "json";
            }

            SECTION("True boolean with protobuf") {
                call.set_isasync(true);
                call.set_ispython(true);
                call.set_istypescript(true);
                conf.serialisation = "proto";
            }

            SECTION("False boolean with protobuf") {
                call.set_isasync(false);
                call.set_ispython(false);
                call.set_istypescript(false);
                conf.serialisation = "proto";
            }

            // Do round trip
            bus.enqueueMessage(call);
            message::Message actual = bus.nextMessage();

            checkMessageEquality(call, actual);
        }

        SECTION("Check reading/ writing function results") {
            bus.setFunctionResult(call);

            // Check result has been written to the right key
            REQUIRE(redis.listLength(call.resultkey()) == 1);

            // Check that some expiry has been set
            long ttl = redis.getTtl(call.resultkey());
            REQUIRE(ttl > 10);

            // Check retrieval method gets the same call out again
            message::Message actualCall2 = bus.getFunctionResult(call.id(), 1);

            checkMessageEquality(call, actualCall2);
        }

        conf.serialisation = originalSerialisation;
    }

    TEST_CASE("Check multithreaded function results", "[scheduler]") {
        cleanSystem();

        int nWorkers = 5;
        int nWorkerMessages = 8;

        int nWaiters = 10;
        int nWaiterMessages = 4;

        // Sanity check
        REQUIRE((nWaiters * nWaiterMessages) == (nWorkers * nWorkerMessages));
        
        std::vector<std::thread> waiterThreads;
        std::vector<std::thread> workerThreads;

        // Create waiters that will submit messages and await their results
        for (int i = 0; i < nWaiters; i++) {
            waiterThreads.emplace_back([nWaiterMessages] {
                Scheduler &sch = scheduler::getScheduler();
                GlobalMessageBus &bus = getGlobalMessageBus();

                message::Message msg = util::messageFactory("demo", "echo");

                // Put invocation on local queue and await global result
                for(int m = 0; m < nWaiterMessages; m++) {
                    sch.enqueueMessage(msg);
                    bus.getFunctionResult(msg.id(), 5000);
                }
            });
        }

        // Create workers that will dequeue messages and set success
        for (int i = 0; i < nWorkers; i++) {
            workerThreads.emplace_back([nWorkerMessages] {
                Scheduler &sch = scheduler::getScheduler();
                GlobalMessageBus &bus = getGlobalMessageBus();

                message::Message dummyMsg = util::messageFactory("demo", "echo");
                const std::shared_ptr<InMemoryMessageQueue> &queue = sch.getFunctionQueue(dummyMsg);

                // Listen to local queue, set result on global bus
                for(int m = 0; m < nWorkerMessages; m++) {
                    message::Message msg = queue->dequeue(5000);
                    bus.setFunctionResult(msg);
                }
            });
        }

        // Wait for all the threads to finish
        for(auto &w : waiterThreads) {
            if(w.joinable()) {
                w.join();
            }
        }

        for(auto &w : workerThreads) {
            if(w.joinable()) {
                w.join();
            }
        }

        // If we get here then things work properly
    }

    TEST_CASE("Check getting function status", "[scheduler]") {
        cleanSystem();
        
        GlobalMessageBus &bus = getGlobalMessageBus();
        
        // Create a message

        std::string expectedOutput;
        int expectedReturnValue = 0;
        message::Message_MessageType expectedType;
        std::string expectedNodeId = util::getNodeId();

        message::Message msg;
        SECTION("Running") {
            msg = util::messageFactory("demo", "echo");
            expectedReturnValue = 0;
            expectedType = message::Message_MessageType_EMPTY;
            expectedNodeId = "";
        }

        SECTION("Failure") {
            msg = util::messageFactory("demo", "echo");

            expectedOutput = "I have failed";
            msg.set_outputdata(expectedOutput);
            msg.set_returnvalue(1);
            bus.setFunctionResult(msg);

            expectedReturnValue = 1;
            expectedType = message::Message_MessageType_CALL;
        }

        SECTION("Success") {
            msg = util::messageFactory("demo", "echo");

            expectedOutput = "I have succeeded";
            msg.set_outputdata(expectedOutput);
            msg.set_returnvalue(0);
            bus.setFunctionResult(msg);

            expectedReturnValue = 0;
            expectedType = message::Message_MessageType_CALL;
        }

        // Check status when nothing has been written 
        const message::Message result = bus.getFunctionResult(msg.id(), 0);

        REQUIRE(result.returnvalue() == expectedReturnValue);
        REQUIRE(result.type() == expectedType);
        REQUIRE(result.outputdata() == expectedOutput);
        REQUIRE(result.executednode() == expectedNodeId);
    }
}