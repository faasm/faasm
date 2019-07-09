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
        message::Message snapCall = util::messageFactory("demo", "snapshot");

        execFunction(snapCall);
        message::Message snapResult = globalBus.getFunctionResult(snapCall.id());
        REQUIRE(snapResult.success());

        // Execute the restore call
        message::Message restoreCall = util::messageFactory("demo", "restore");
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
