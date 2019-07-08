#include <catch/catch.hpp>

#include "utils.h"

#include <worker/WorkerThreadPool.h>
#include <worker/WorkerThread.h>
#include <wasm/WasmModule.h>
#include <util/bytes.h>


namespace tests {
    void checkIntegerOutput(int expectedInt, message::Message &msg) {
        auto intBytes = reinterpret_cast<uint8_t *>(&expectedInt);

        const std::vector<uint8_t> expected(intBytes, intBytes + sizeof(int));
        const std::vector<uint8_t> &actual = util::stringToBytes(msg.outputdata());

        REQUIRE(actual == expected);
    }

    TEST_CASE("Test running snapshot/ restore functions", "[pool]") {
        cleanSystem();

        scheduler::GlobalMessageBus &globalBus = scheduler::getGlobalMessageBus();

        // Execute the snap call
        message::Message snapCall;
        snapCall.set_user("demo");
        snapCall.set_function("snapshot");
        snapCall.set_resultkey("test_snapshot");

        execFunction(snapCall);
        message::Message snapResult = globalBus.getFunctionResult(snapCall.id());
        REQUIRE(snapResult.success());

        // Execute the restore call
        message::Message restoreCall;
        restoreCall.set_user("demo");
        restoreCall.set_function("restore");
        restoreCall.set_resultkey("test_restore");

        execFunction(restoreCall);
        message::Message restoreResult = globalBus.getFunctionResult(restoreCall.id());
        REQUIRE(restoreResult.success());

        // Check outputs
        int expectedSnapOutput = 4;
        int expectedRestoreOutput = 7;

        checkIntegerOutput(expectedSnapOutput, snapResult);
        checkIntegerOutput(expectedRestoreOutput, restoreResult);
    }
}
