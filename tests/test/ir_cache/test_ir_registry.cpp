#include <catch/catch.hpp>

#include <util/bytes.h>
#include <util/func.h>
#include <util/files.h>

#include <ir_cache/IRModuleCache.h>
#include <storage/FileLoader.h>

namespace tests {
    void checkObjCode(const Runtime::ModuleRef moduleRef, const std::string &path) {
        const std::vector<uint8_t> fileBytes = util::readFileToBytes(path);
        const std::vector<U8> moduleBytes = Runtime::getObjectCode(moduleRef);

        REQUIRE(moduleBytes == fileBytes);
    }

    TEST_CASE("Test main module caching", "[wasm]") {
        wasm::IRModuleCache &registry = wasm::getIRModuleCache();

        // Two valid functions
        std::string user = "demo";
        std::string funcA = "echo";
        std::string funcB = "x2";

        faabric::Message msgA = util::messageFactory(user, funcA);
        faabric::Message msgB = util::messageFactory(user, funcB);

        // Get once via both means
        IR::Module &moduleRefA1 = registry.getModule(user, funcA, "");
        Runtime::ModuleRef objRefA1 = registry.getCompiledModule(user, funcA, "");
        IR::Module &moduleRefB1 = registry.getModule(user, funcB, "");
        Runtime::ModuleRef objRefB1 = registry.getCompiledModule(user, funcB, "");

        // And again
        IR::Module &moduleRefA2 = registry.getModule(user, funcA, "");
        Runtime::ModuleRef objRefA2 = registry.getCompiledModule(user, funcA, "");
        IR::Module &moduleRefB2 = registry.getModule(user, funcB, "");
        Runtime::ModuleRef objRefB2 = registry.getCompiledModule(user, funcB, "");

        // Sanity check the modules
        REQUIRE(!moduleRefA1.exports.empty());
        REQUIRE(!moduleRefB1.exports.empty());
        REQUIRE(moduleRefA1.memories.defs[0].type.size.max == MAX_MEMORY_PAGES);
        REQUIRE(moduleRefA1.tables.defs[0].type.size.max == MAX_TABLE_SIZE);

        // Check features enabled
        REQUIRE(moduleRefA1.featureSpec.simd);
        REQUIRE(moduleRefA2.featureSpec.simd);
        REQUIRE(moduleRefB1.featureSpec.simd);
        REQUIRE(moduleRefB2.featureSpec.simd);

        REQUIRE(moduleRefA1.featureSpec.atomics);
        REQUIRE(moduleRefA2.featureSpec.atomics);
        REQUIRE(moduleRefB1.featureSpec.atomics);
        REQUIRE(moduleRefB2.featureSpec.atomics);

        // Check references are equal
        REQUIRE(std::addressof(moduleRefA1) == std::addressof(moduleRefA2));
        REQUIRE(objRefA1 == objRefA2);
        REQUIRE(std::addressof(moduleRefB1) == std::addressof(moduleRefB2));
        REQUIRE(objRefB1 == objRefB2);

        // Check different module references are different
        REQUIRE(std::addressof(moduleRefA1) != std::addressof(moduleRefB1));
        REQUIRE(objRefA1 != objRefB1);

        const std::string objPathA = util::getFunctionObjectFile(msgA);
        const std::string objPathB = util::getFunctionObjectFile(msgB);

        checkObjCode(objRefA1, objPathA);
        checkObjCode(objRefB1, objPathB);
    }

    TEST_CASE("Test shared library caching", "[wasm]") {
        wasm::IRModuleCache &registry = wasm::getIRModuleCache();

        std::string user = "demo";
        std::string func = "echo";

        // Must get main module first
        registry.getModule(user, func, "");

        // Two valid shared libs
        std::string pathA = "/usr/local/faasm/runtime_root/lib/python3.7/site-packages/numpy/core/multiarray.so";
        std::string pathB = "/usr/local/faasm/runtime_root/lib/python3.7/site-packages/numpy/core/umath.so";

        // Once
        IR::Module &refA1 = registry.getModule(user, func, pathA);
        Runtime::ModuleRef objRefA1 = registry.getCompiledModule(user, func, pathA);
        IR::Module &refB1 = registry.getModule(user, func, pathB);
        Runtime::ModuleRef objRefB1 = registry.getCompiledModule(user, func, pathB);

        // Again
        IR::Module &refA2 = registry.getModule(user, func, pathA);
        Runtime::ModuleRef objRefA2 = registry.getCompiledModule(user, func, pathA);
        IR::Module &refB2 = registry.getModule(user, func, pathB);
        Runtime::ModuleRef objRefB2 = registry.getCompiledModule(user, func, pathB);

        // Sanity checks
        REQUIRE(!refA1.exports.empty());
        REQUIRE(!refB1.exports.empty());

        // Check features enabled
        REQUIRE(refA1.featureSpec.simd);
        REQUIRE(refA2.featureSpec.simd);
        REQUIRE(refB1.featureSpec.simd);
        REQUIRE(refB2.featureSpec.simd);

        REQUIRE(refA1.featureSpec.atomics);
        REQUIRE(refA2.featureSpec.atomics);
        REQUIRE(refB1.featureSpec.atomics);
        REQUIRE(refB2.featureSpec.atomics);

        // Check references are equal
        REQUIRE(std::addressof(refA1) == std::addressof(refA2));
        REQUIRE(objRefA1 == objRefA2);
        REQUIRE(std::addressof(refB1) == std::addressof(refB2));
        REQUIRE(objRefB1 == objRefB2);

        // Check different module references are different
        REQUIRE(std::addressof(refA1) != std::addressof(refB1));
        REQUIRE(objRefA1 != objRefB1);

        // Check object code loaded matches file
        std::string objPathA = util::getSharedObjectObjectFile(pathA);
        std::string objPathB = util::getSharedObjectObjectFile(pathB);
        
        checkObjCode(objRefA1, objPathA);
        checkObjCode(objRefB1, objPathB);
    }
}