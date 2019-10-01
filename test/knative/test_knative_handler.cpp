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

        message::Message expectedCall;

        SECTION("C/C++") {
            user = "demo";
            function = "echo";

            SECTION("With input") {
                call.set_inputdata("foobar");
            }
            SECTION("No input") {

            }

            expectedCall = call;
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

            expectedCall = call;
        }
        SECTION("Python") {
            user = "python";
            function = "hello";
            call.set_ispython(true);
        }

        call.set_user(user);
        call.set_function(function);

        const std::string &requestStr = util::messageToJson(call);

        // Handle the function
        knative::KnativeHandler handler;
        handler.handleFunction(requestStr);

        // Check function count has increased and bind message sent
        expectedCall = call;
        if (user == "python") {
            util::convertMessageToPython(expectedCall);
        }

        scheduler::Scheduler &sch = scheduler::getScheduler();
        REQUIRE(sch.getFunctionQueueLength(expectedCall) == 1);
        REQUIRE(sch.getBindQueue()->size() == 1);
        message::Message actual = sch.getBindQueue()->dequeue();

        REQUIRE(actual.user() == expectedCall.user());
        REQUIRE(actual.function() == expectedCall.function());
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
}