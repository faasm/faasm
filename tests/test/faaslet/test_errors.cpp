#include <catch/catch.hpp>

#include "utils.h"

#include <faaslet/FaasletPool.h>
#include <faabric/scheduler/InMemoryMessageQueue.h>

using namespace faaslet;

namespace tests {
    void execErrorFunction(faabric::Message &call) {
        FaasletPool pool(1);
        Faaslet w(1);

        faabric::scheduler::Scheduler &sch = faabric::scheduler::getScheduler();
        sch.callFunction(call);

        // Bind message
        w.processNextMessage();

        // Execution message
        w.processNextMessage();
    }

    void checkError(const std::string &funcName, const std::string &expectedMsg) {
        cleanSystem();

        faabric::Message call = faabric::util::messageFactory("errors", funcName);

        execErrorFunction(call);

        // Get result
        faabric::scheduler::Scheduler &sch = faabric::scheduler::getScheduler();
        faabric::Message result = sch.getFunctionResult(call.id(), 1);
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