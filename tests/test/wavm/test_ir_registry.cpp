#include <catch2/catch.hpp>

#include <faabric/util/bytes.h>
#include <faabric/util/files.h>
#include <faabric/util/func.h>

#include <storage/FileLoader.h>
#include <wavm/IRModuleCache.h>

namespace tests {
void checkObjCode(const Runtime::ModuleRef moduleRef, const std::string& path)
{
    const std::vector<uint8_t> fileBytes = faabric::util::readFileToBytes(path);
    const std::vector<U8> moduleBytes = Runtime::getObjectCode(moduleRef);

    REQUIRE(moduleBytes == fileBytes);
}

TEST_CASE("Test main module caching", "[wasm]")
{
    wasm::IRModuleCache& registry = wasm::getIRModuleCache();

    // Two valid functions
    std::string user = "demo";
    std::string funcA = "echo";
    std::string funcB = "x2";

    faabric::Message msgA = faabric::util::messageFactory(user, funcA);
    faabric::Message msgB = faabric::util::messageFactory(user, funcB);

    // Get once via both means
    IR::Module& moduleRefA1 = registry.getModule(user, funcA, "");
    Runtime::ModuleRef objRefA1 = registry.getCompiledModule(user, funcA, "");
    IR::Module& moduleRefB1 = registry.getModule(user, funcB, "");
    Runtime::ModuleRef objRefB1 = registry.getCompiledModule(user, funcB, "");

    // Check they are reported as cached
    REQUIRE(registry.isModuleCached(user, funcA, ""));
    REQUIRE(registry.isModuleCached(user, funcB, ""));
    REQUIRE(registry.isCompiledModuleCached(user, funcA, ""));
    REQUIRE(registry.isCompiledModuleCached(user, funcB, ""));

    // And again
    IR::Module& moduleRefA2 = registry.getModule(user, funcA, "");
    Runtime::ModuleRef objRefA2 = registry.getCompiledModule(user, funcA, "");
    IR::Module& moduleRefB2 = registry.getModule(user, funcB, "");
    Runtime::ModuleRef objRefB2 = registry.getCompiledModule(user, funcB, "");

    // Sanity check the modules
    REQUIRE(!moduleRefA1.exports.empty());
    REQUIRE(!moduleRefB1.exports.empty());
    REQUIRE(moduleRefA1.memories.defs[0].type.size.max == MAX_MEMORY_PAGES);
    REQUIRE(moduleRefA1.tables.defs[0].type.size.max == MAX_TABLE_SIZE);

    // Check features are as expected
    REQUIRE(moduleRefA1.featureSpec.simd);
    REQUIRE(moduleRefA2.featureSpec.simd);
    REQUIRE(moduleRefB1.featureSpec.simd);
    REQUIRE(moduleRefB2.featureSpec.simd);

    // Check references are equal
    REQUIRE(std::addressof(moduleRefA1) == std::addressof(moduleRefA2));
    REQUIRE(objRefA1 == objRefA2);
    REQUIRE(std::addressof(moduleRefB1) == std::addressof(moduleRefB2));
    REQUIRE(objRefB1 == objRefB2);

    // Check different module references are different
    REQUIRE(std::addressof(moduleRefA1) != std::addressof(moduleRefB1));
    REQUIRE(objRefA1 != objRefB1);

    const std::string objPathA = faabric::util::getFunctionObjectFile(msgA);
    const std::string objPathB = faabric::util::getFunctionObjectFile(msgB);

    checkObjCode(objRefA1, objPathA);
    checkObjCode(objRefB1, objPathB);
}

TEST_CASE("Test shared library caching", "[wasm]")
{
    wasm::IRModuleCache& registry = wasm::getIRModuleCache();

    std::string user = "demo";
    std::string func = "echo";

    // Must get main module first
    registry.getModule(user, func, "");

    // Two valid shared libs
    std::string pathA = "/usr/local/faasm/runtime_root/lib/python3.8/"
                        "site-packages/numpy/core/_multiarray_umath.so";
    std::string pathB = "/usr/local/faasm/runtime_root/lib/python3.8/"
                        "site-packages/numpy/linalg/_umath_linalg.so";

    // Once
    IR::Module& refA1 = registry.getModule(user, func, pathA);
    Runtime::ModuleRef objRefA1 = registry.getCompiledModule(user, func, pathA);
    IR::Module& refB1 = registry.getModule(user, func, pathB);
    Runtime::ModuleRef objRefB1 = registry.getCompiledModule(user, func, pathB);

    // Check they are reported as cached
    REQUIRE(registry.isModuleCached(user, func, pathA));
    REQUIRE(registry.isModuleCached(user, func, pathB));
    REQUIRE(registry.isCompiledModuleCached(user, func, pathA));
    REQUIRE(registry.isCompiledModuleCached(user, func, pathB));

    // Again
    IR::Module& refA2 = registry.getModule(user, func, pathA);
    Runtime::ModuleRef objRefA2 = registry.getCompiledModule(user, func, pathA);
    IR::Module& refB2 = registry.getModule(user, func, pathB);
    Runtime::ModuleRef objRefB2 = registry.getCompiledModule(user, func, pathB);

    // Sanity checks
    REQUIRE(!refA1.exports.empty());
    REQUIRE(!refB1.exports.empty());

    // Check features enabled
    REQUIRE(refA1.featureSpec.simd);
    REQUIRE(refA2.featureSpec.simd);
    REQUIRE(refB1.featureSpec.simd);
    REQUIRE(refB2.featureSpec.simd);

    // Check references are equal
    REQUIRE(std::addressof(refA1) == std::addressof(refA2));
    REQUIRE(objRefA1 == objRefA2);
    REQUIRE(std::addressof(refB1) == std::addressof(refB2));
    REQUIRE(objRefB1 == objRefB2);

    // Check different module references are different
    REQUIRE(std::addressof(refA1) != std::addressof(refB1));
    REQUIRE(objRefA1 != objRefB1);

    // Check object code loaded matches file
    std::string objPathA = faabric::util::getSharedObjectObjectFile(pathA);
    std::string objPathB = faabric::util::getSharedObjectObjectFile(pathB);

    checkObjCode(objRefA1, objPathA);
    checkObjCode(objRefB1, objPathB);
}

TEST_CASE("Test IR cache clearing", "[wasm]")
{
    wasm::IRModuleCache& registry = wasm::getIRModuleCache();

    // Function and shared lib
    std::string user = "demo";
    std::string func = "echo";
    std::string libPath = "/usr/local/faasm/runtime_root/lib/python3.8/"
                          "site-packages/numpy/core/_multiarray_umath.so";

    registry.getModule(user, func, "");
    registry.getCompiledModule(user, func, "");

    registry.getModule(user, func, libPath);
    registry.getCompiledModule(user, func, libPath);

    // Check they are reported as cached
    REQUIRE(registry.isModuleCached(user, func, ""));
    REQUIRE(registry.isCompiledModuleCached(user, func, ""));
    REQUIRE(registry.isModuleCached(user, func, libPath));
    REQUIRE(registry.isCompiledModuleCached(user, func, libPath));

    registry.clear();

    REQUIRE(!registry.isModuleCached(user, func, ""));
    REQUIRE(!registry.isCompiledModuleCached(user, func, ""));
    REQUIRE(!registry.isModuleCached(user, func, libPath));
    REQUIRE(!registry.isCompiledModuleCached(user, func, libPath));
}
}
