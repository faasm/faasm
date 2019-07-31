#include <catch/catch.hpp>
#include <wasm/IRModuleRegistry.h>

#include <util/bytes.h>
#include <util/func.h>
#include <WAVM/IR/Module.h>

namespace tests {
    TEST_CASE("Test main module caching", "[wasm]") {
        wasm::IRModuleRegistry &registry = wasm::getIRModuleRegistry();

        // Two valid functions
        message::Message msgA = util::messageFactory("demo", "echo");
        message::Message msgB = util::messageFactory("demo", "x2");

        // Get once via both means
        IR::Module &moduleRefA1 = registry.getMainModule(msgA);
        Runtime::ModuleRef &objRefA1 = registry.getCompiledModule(msgA);
        IR::Module &moduleRefB1 = registry.getMainModule(msgB);
        Runtime::ModuleRef &objRefB1 = registry.getCompiledModule(msgB);

        // And again
        IR::Module &moduleRefA2 = registry.getMainModule(msgA);
        Runtime::ModuleRef &objRefA2 = registry.getCompiledModule(msgA);
        IR::Module &moduleRefB2 = registry.getMainModule(msgB);
        Runtime::ModuleRef &objRefB2 = registry.getCompiledModule(msgB);

        REQUIRE(!moduleRefA1.exports.empty());
        REQUIRE(!moduleRefB1.exports.empty());

        // Check references are equal
        REQUIRE(std::addressof(moduleRefA1) == std::addressof(moduleRefA2));
        REQUIRE(std::addressof(objRefA1) == std::addressof(objRefA2));
        REQUIRE(std::addressof(moduleRefB1) == std::addressof(moduleRefB2));
        REQUIRE(std::addressof(objRefB1) == std::addressof(objRefB2));

        // Check different module references are different
        REQUIRE(std::addressof(moduleRefA1) != std::addressof(moduleRefB1));
        REQUIRE(std::addressof(objRefA1) != std::addressof(objRefB1));
    }

    TEST_CASE("Test shared library caching", "[wasm]") {
        wasm::IRModuleRegistry &registry = wasm::getIRModuleRegistry();

        // Two valid shared libs
        std::string pathA = "/usr/local/faasm/runtime_root/lib/python3.7/site-packages/numpy/core/multiarray.so";
        std::string pathB = "/usr/local/faasm/runtime_root/lib/python3.7/site-packages/numpy/core/umath.so";

        // Once
        IR::Module &refA1 = registry.getSharedModule(pathA);
        Runtime::ModuleRef &objRefA1 = registry.getCompiledSharedModule(pathA);
        IR::Module &refB1 = registry.getSharedModule(pathB);
        Runtime::ModuleRef &objRefB1 = registry.getCompiledSharedModule(pathB);

        // Again
        IR::Module &refA2 = registry.getSharedModule(pathA);
        Runtime::ModuleRef &objRefA2 = registry.getCompiledSharedModule(pathA);
        IR::Module &refB2 = registry.getSharedModule(pathB);
        Runtime::ModuleRef &objRefB2 = registry.getCompiledSharedModule(pathB);

        REQUIRE(!refA1.exports.empty());
        REQUIRE(!refB1.exports.empty());

        // Check references are equal
        REQUIRE(std::addressof(refA1) == std::addressof(refA2));
        REQUIRE(std::addressof(objRefA1) == std::addressof(objRefA2));
        REQUIRE(std::addressof(refB1) == std::addressof(refB2));
        REQUIRE(std::addressof(objRefB1) == std::addressof(objRefB2));

        // Check different module references are different
        REQUIRE(std::addressof(refA1) != std::addressof(refB1));
        REQUIRE(std::addressof(objRefA1) != std::addressof(objRefB1));

    }
}