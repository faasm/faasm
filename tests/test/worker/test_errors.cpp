#include <catch/catch.hpp>

#include "utils.h"

#include <worker/WorkerThreadPool.h>
#include <worker/WorkerThread.h>
#include <scheduler/InMemoryMessageQueue.h>

using namespace worker;

namespace tests {
    void execErrorFunction(message::Message &call) {
        WorkerThreadPool pool(1);
        WorkerThread w(1);

        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.callFunction(call);

        // Bind message
        w.processNextMessage();

        // Execution message
        w.processNextMessage();
    }

    void checkError(const std::string &funcName, const std::string &expectedMsg) {
        cleanSystem();

        message::Message call = util::messageFactory("errors", funcName);

        execErrorFunction(call);

        // Get result
        scheduler::GlobalMessageBus &messageQueue = scheduler::getGlobalMessageBus();

        message::Message result = messageQueue.getFunctionResult(call.id(), 1);
        REQUIRE(result.returnvalue() > 0);

        if(expectedMsg.empty()) {
            return;
        }

        const std::string actualOutput = result.outputdata();
        bool messageIsFound = false;
        if (actualOutput.find(expectedMsg) != std::string::npos) {
            messageIsFound = true;
        }

        if (!messageIsFound) {
            printf("%s not found in %s\n", expectedMsg.c_str(), actualOutput.c_str());
        }

        REQUIRE(messageIsFound);
    }

    TEST_CASE("Test non-zero return code is error", "[wasm]") {
        checkError("ret_one", "Call failed (return value=1)");
    }

//    TEST_CASE("Test munmapped memory not usable", "[wasm]") {
//        checkError("munmap", "");
//    }
}