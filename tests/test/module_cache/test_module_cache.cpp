#include "utils.h"
#include <catch2/catch.hpp>

#include <faabric/util/func.h>
#include <module_cache/WasmModuleCache.h>

namespace tests {
TEST_CASE("Test creating zygotes", "[zygote]")
{
    cleanSystem();

    faabric::Message msgA = faabric::util::messageFactory("demo", "chain");
    faabric::Message msgB = faabric::util::messageFactory("demo", "chain");

    // Want to check things with chained calls, so need to fake up input to a
    // chained func
    msgA.set_idx(1);
    int input[3] = { 1, 2, 3 };
    msgA.set_inputdata(BYTES(input), 3 * sizeof(int));

    module_cache::WasmModuleCache& registry =
      module_cache::getWasmModuleCache();
    wasm::WasmModule& moduleA = registry.getCachedModule(msgA);
    wasm::WasmModule& moduleB = registry.getCachedModule(msgB);

    // Check modules are the same
    REQUIRE(std::addressof(moduleA) == std::addressof(moduleB));
    REQUIRE(moduleA.isBound());

    // Execute the function normally and make sure zygote is not used directly
    faaslet::Faaslet faaslet = execFunction(msgA);
    REQUIRE(faaslet.isBound());
    REQUIRE(std::addressof(moduleA) != std::addressof(*faaslet.module));
}
}
