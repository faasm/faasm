#include <catch/catch.hpp>

#include "utils.h"
#include <knative/KnativeHandler.h>

#include <scheduler/GlobalMessageBus.h>
#include <util/json.h>

using namespace Pistache;

namespace tests {
    TEST_CASE("Test valid knative invocations", "[knative]") {
        cleanSystem();

        // Note - must be async to avoid needing a result
        message::Message call;
        call.set_isasync(true);
        std::string user;
        std::string function;

        SECTION("C/C++") {
            user = "demo";
            function = "echo";

            SECTION("With input") {
                call.set_inputdata("foobar");
            }
            SECTION("No input") {

            }
        }
        SECTION("Typescript") {
            user = "ts";
            function = "echo";
            call.set_istypescript(true);

            SECTION("With input") {
                call.set_inputdata("foobar");
            }
            SECTION("No input") {

            }
        }
        SECTION("Python") {
            user = PYTHON_USER;
            function = PYTHON_FUNC;
            call.set_pythonuser("python");
            call.set_pythonfunction("hello");
            call.set_ispython(true);
        }

        call.set_user(user);
        call.set_function(function);

        const std::string &requestStr = util::messageToJson(call);

        // Handle the function
        knative::KnativeHandler handler;
        const std::string responseStr = handler.handleFunction(requestStr);

        // Check function count has increased and bind message sent
        scheduler::Scheduler &sch = scheduler::getScheduler();
        REQUIRE(sch.getFunctionInFlightCount(call) == 1);
        REQUIRE(sch.getBindQueue()->size() == 1);

        message::Message actualBind = sch.getBindQueue()->dequeue();
        REQUIRE(actualBind.user() == call.user());
        REQUIRE(actualBind.function() == call.function());

        // Check actual call has right details including the ID returned to the caller
        message::Message actualCall = sch.getFunctionQueue(call)->dequeue();
        REQUIRE(actualCall.user() == call.user());
        REQUIRE(actualCall.function() == call.function());
        REQUIRE(actualCall.id() == std::stoi(responseStr));
    }

    TEST_CASE("Test empty knative invocation", "[knative]") {
        knative::KnativeHandler handler;
        std::string actual = handler.handleFunction("");

        REQUIRE(actual == "Empty request");
    }

    TEST_CASE("Test empty JSON knative invocation", "[knative]") {
        message::Message call;
        call.set_isasync(true);

        std::string expected;

        SECTION("Empty user") {
            expected = "Empty user";
            call.set_function("echo");
        }

        SECTION("Empty function") {
            expected = "Empty function";
            call.set_user("demo");
        }

        knative::KnativeHandler handler;
        const std::string &requestStr = util::messageToJson(call);
        std::string actual = handler.handleFunction(requestStr);

        REQUIRE(actual == expected);
    }

    TEST_CASE("Check getting function status from knative", "[knative]") {
        cleanSystem();

        scheduler::GlobalMessageBus &bus = scheduler::getGlobalMessageBus();

        // Create a message
        message::Message msg = util::messageFactory("demo", "echo");

        std::string expectedOutput;
        SECTION("Running") {
            expectedOutput = "RUNNING";
        }

        SECTION("Failure") {
            std::string errorMsg = "I have failed";
            msg.set_outputdata(errorMsg);
            msg.set_returnvalue(1);
            bus.setFunctionResult(msg);

            expectedOutput = "FAILED: " + errorMsg;
        }

        SECTION("Success") {
            std::string errorMsg = "I have succeeded";
            msg.set_outputdata(errorMsg);
            msg.set_returnvalue(0);
            bus.setFunctionResult(msg);

            expectedOutput = "SUCCESS: " + errorMsg;
        }

        msg.set_isstatusrequest(true);

        knative::KnativeHandler handler;
        const std::string &requestStr = util::messageToJson(msg);
        std::string actual = handler.handleFunction(requestStr);

        REQUIRE(actual == expectedOutput);

    }
    
    void checkFlushMessageShared(const std::string &node, const message::Message &msg) {
        scheduler::SharingMessageBus &sharingBus = scheduler::SharingMessageBus::getInstance();
        const message::Message actual = sharingBus.nextMessageForNode(node);
        REQUIRE(actual.isflushrequest());
    }
    
    TEST_CASE("Test broadcasting flush message", "[knative]") {
        cleanSystem();

        // Set up some dummy nodes and add to global set
        std::string thisNode = util::getNodeId();
        std::string nodeA = "node_a";
        std::string nodeB = "node_b";

        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.addNodeToGlobalSet(nodeA);
        sch.addNodeToGlobalSet(nodeB);
        
        message::Message msg;
        msg.set_isflushrequest(true);
        
        knative::KnativeHandler handler;
        const std::string &requestStr = util::messageToJson(msg);
        std::string actual = handler.handleFunction(requestStr);

        checkFlushMessageShared(thisNode, msg);
        checkFlushMessageShared(nodeA, msg);
        checkFlushMessageShared(nodeB, msg);
    }
}