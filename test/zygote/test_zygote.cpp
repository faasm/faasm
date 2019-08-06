#include <catch/catch.hpp>
#include "utils.h"

#include <util/func.h>
#include <zygote/ZygoteRegistry.h>


namespace tests {
    TEST_CASE("Test creating zygotes", "[zygote]") {
        cleanSystem();

        message::Message msg = util::messageFactory("demo", "echo");
        
        zygote::ZygoteRegistry &registry = zygote::getZygoteRegistry();
        wasm::WasmModule &moduleA = registry.getZygote(msg);
        wasm::WasmModule &moduleB = registry.getZygote(msg);
        
        // Check modules are the same
        REQUIRE(std::addressof(moduleA) == std::addressof(moduleB));
        REQUIRE(moduleA.isBound());
        
        // Execute the function normally and make sure zygote is not used directly
        worker::WorkerThread workerThread = execFunction(msg);
        REQUIRE(workerThread.isBound());
        REQUIRE(std::addressof(moduleA) != std::addressof(*workerThread.module));
    }
}