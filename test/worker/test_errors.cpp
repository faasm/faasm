#include <catch/catch.hpp>

#include <worker/worker.h>
#include "utils.h"

using namespace worker;

namespace tests {
    void execErrorFunction(message::Message &call) {
        WorkerThreadPool pool;
        WorkerThread w(pool, 1, 1);

        infra::Scheduler::callFunction(call);

        // Bind message
        w.processNextMessage();

        // Execution message
        w.processNextMessage();
    }

    void checkError(const std::string &funcName, const std::string &expectedMsg) {
        infra::Redis &redisQueue = infra::Redis::getQueue();
        redisQueue.flushAll();

        message::Message call;
        call.set_user("errors");
        call.set_function(funcName);
        call.set_resultkey("error_test");

        execErrorFunction(call);

        // Get result
        message::Message result = redisQueue.getFunctionResult(call);
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