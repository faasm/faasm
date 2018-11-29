#include <catch/catch.hpp>

#include <worker/worker.h>

using namespace worker;

namespace tests {
    static infra::Redis redis;

    void execErrorFunction(message::Message &call) {
        WorkerThread w(1);
        w.bindToFunction(call);

        infra::Scheduler::callFunction(call);

        w.processNextMessage();
    }

    void checkError(const std::string &funcName, const std::string &expectedMsg) {
        redis.flushAll();

        message::Message call;
        call.set_user("errors");
        call.set_function(funcName);
        call.set_resultkey("error_test");

        execErrorFunction(call);

        // Get result
        message::Message result = redis.getFunctionResult(call);
        REQUIRE(!result.success());

        const std::string actualOutput = result.outputdata();
        bool messageIsFound = false;
        if (actualOutput.find(expectedMsg) != std::string::npos) {
            messageIsFound = true;
        }

        if (!messageIsFound) {
            std::cout << expectedMsg << " not found in " << actualOutput.c_str() << std::endl;
        }

        REQUIRE(messageIsFound);
    }

    TEST_CASE("Test opening blocked path", "[wasm]") {
        checkError("open_blocked", "Attempt to open invalid file");
    }

    TEST_CASE("Test connecting to socket not owned by function", "[wasm]") {
        checkError("socket", "fd not owned by this function");
    }

    TEST_CASE("Test polling multiple fds not owned by function", "[wasm]") {
        checkError("multi_poll", "fd not owned by this function");
    }

    TEST_CASE("Test polling stdin", "[wasm]") {
        checkError("stdin_poll", "Attempt to interact with stdin");
    }
}