#include <catch/catch.hpp>
#include "utils.h"

#include <util/func.h>
#include <zygote/ZygoteRegistry.h>


namespace tests {
    TEST_CASE("Test creating zygotes", "[zygote]") {
        cleanSystem();

        message::Message msgA = util::messageFactory("demo", "echo");
        message::Message msgB = util::messageFactory("demo", "echo");

        zygote::ZygoteRegistry &registry = zygote::getZygoteRegistry();
        wasm::WasmModule &moduleA = registry.getZygote(msgA);
        wasm::WasmModule &moduleB = registry.getZygote(msgB);

        // Check modules are the same
        REQUIRE(std::addressof(moduleA) == std::addressof(moduleB));
        REQUIRE(moduleA.isBound());

        // Execute the function normally and make sure zygote is not used directly
        worker::WorkerThread workerThread = execFunction(msgA);
        REQUIRE(workerThread.isBound());
        REQUIRE(std::addressof(moduleA) != std::addressof(*workerThread.module));
    }
}