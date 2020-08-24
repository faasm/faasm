#include <catch/catch.hpp>

#include "utils.h"

#include <util/environment.h>

#include <faaslet/FaasletPool.h>
#include <faaslet/Faaslet.h>
#include <emulator/emulator.h>
#include <util/bytes.h>

using namespace faaslet;

namespace tests {
    TEST_CASE("Test repeat invocation with state", "[faaslet]") {
        // Set up the function call
        faabric::Message call = util::messageFactory("demo", "increment");
        setEmulatedMessage(call);

        // Call function
        FaasletPool pool(1);
        Faaslet w(1);

        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.callFunction(call);

        // Bind and exec
        w.processNextMessage();
        w.processNextMessage();

        // Check result
        faabric::Message resultA = sch.getFunctionResult(call.id(), 1);
        REQUIRE(resultA.returnvalue() == 0);
        REQUIRE(resultA.outputdata() == "Counter: 001");

        // Call the function a second time, the state should have been incremented
        call.set_id(0);
        util::setMessageId(call);
        setEmulatedMessage(call);

        sch.callFunction(call);
        w.processNextMessage();

        faabric::Message resultB = sch.getFunctionResult(call.id(), 1);
        REQUIRE(resultB.returnvalue() == 0);
        REQUIRE(resultB.outputdata() == "Counter: 002");
    }

    void checkStateExample(const std::string &funcName, const std::string &keyName,
                           const std::vector<uint8_t> &expectedOutput,
                           const std::vector<uint8_t> &expectedState) {
        cleanSystem();

        // Set up the function call
        faabric::Message call = util::messageFactory("demo", funcName);
        setEmulatedMessage(call);

        // Call function
        FaasletPool pool(1);
        Faaslet w(1);

        scheduler::Scheduler &sch = scheduler::getScheduler();
        sch.callFunction(call);

        // Bind and exec
        w.processNextMessage();
        w.processNextMessage();

        // Check result
        faabric::Message result = sch.getFunctionResult(call.id(), 1);
        REQUIRE(result.returnvalue() == 0);
        std::vector<uint8_t> outputBytes = util::stringToBytes(result.outputdata());

        REQUIRE(outputBytes == expectedOutput);

        const std::shared_ptr<state::StateKeyValue> &kv = state::getGlobalState().getKV("demo", keyName, 0);
        REQUIRE(kv->size() == expectedState.size());
        std::vector<uint8_t> actual(kv->size(), 0);
        kv->get(actual.data());
        REQUIRE(actual == expectedState);
    }

    TEST_CASE("Test asynchronous state", "[faaslet]") {
        checkStateExample(
                "state_async",
                "state_async_example",
                {1, 1, 1, 1},
                {3, 2, 1, 0}
        );
    }

    TEST_CASE("Test offset state", "[faaslet]") {
        checkStateExample(
                "state_offset",
                "state_offset_example",
                {5, 5, 6, 6, 4},
                {5, 5, 6, 6, 4, 5, 6}
        );
    }

    TEST_CASE("Test state size", "[faaslet]") {
        faabric::Message msg = util::messageFactory("demo", "state_size");
        execFunction(msg);
    }

    TEST_CASE("Test shared state write pointers", "[faaslet]") {
        // Run the function to write
        checkCallingFunctionGivesBoolOutput("demo", "state_shared_write", true);

        // Run the function to read
        checkCallingFunctionGivesBoolOutput("demo", "state_shared_read", true);
    }

    TEST_CASE("Test shared state offset pointers", "[faaslet]") {
        // Run the function to write
        checkCallingFunctionGivesBoolOutput("demo", "state_shared_write_offset", true);

        // Run the function to read
        checkCallingFunctionGivesBoolOutput("demo", "state_shared_read_offset", true);
    }

    TEST_CASE("Test writing file to state", "[faaslet]") {
        cleanSystem();
        faabric::Message msg = util::messageFactory("demo", "state_file");
        execFunction(msg);
    }
}