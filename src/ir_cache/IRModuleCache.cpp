#include "IRModuleCache.h"

#include <faabric/util/locks.h>
#include <faabric/util/logging.h>

#include <WAVM/IR/Module.h>
#include <WAVM/IR/Types.h>
#include <WAVM/WASM/WASM.h>
#include <WAVM/WASTParse/WASTParse.h>

#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <storage/FileLoader.h>

namespace wasm {
IRModuleCache::IRModuleCache()
  : conf(faabric::util::getSystemConfig())
{}

IRModuleCache& getIRModuleCache()
{
    static IRModuleCache r;
    return r;
}

IR::Module& IRModuleCache::getModuleFromMap(const std::string& key)
{
    // Here we switch on module features that must always be used.
    // TODO avoid doing this every time and set on construction
    IR::Module& module = moduleMap[key];
    module.featureSpec.simd = true;
    return module;
}

std::string getModuleKey(const std::string& user,
                         const std::string& func,
                         const std::string& path)
{
    std::string key = user + "_" + func + "_" + path;
    return key;
}

int IRModuleCache::getModuleCount(const std::string& key)
{
    faabric::util::SharedLock lock(registryMutex);
    return moduleMap.count(key);
}

int IRModuleCache::getCompiledModuleCount(const std::string& key)
{
    faabric::util::SharedLock lock(registryMutex);
    return compiledModuleMap.count(key);
}

IR::Module& IRModuleCache::getModule(const std::string& user,
                                     const std::string& func,
                                     const std::string& path)
{
    /*
     * Note that shared modules are currently only shared in memory across
     * instances of the *same* function.  If two different functions both
     * load shared module A, it will be loaded into memory twice.
     *
     * This is currently only to do with needing to modify the table
     * definition to fit with the importing main module, something which can
     * probably be fixed.
     *
     * TODO - implement this proper sharing of modules
     */

    if (path.empty()) {
        return this->getMainModule(user, func);
    } else {
        return this->getSharedModule(user, func, path);
    }
}

Runtime::ModuleRef IRModuleCache::getCompiledModule(const std::string& user,
                                                    const std::string& func,
                                                    const std::string& path)
{
    if (path.empty()) {
        return this->getCompiledMainModule(user, func);
    } else {
        return this->getCompiledSharedModule(user, func, path);
    }
}

U64 IRModuleCache::getSharedModuleTableSize(const std::string& user,
                                            const std::string& func,
                                            const std::string& path)
{
    const std::string key = getModuleKey(user, func, path);
    return originalTableSizes[key];
}

Runtime::ModuleRef IRModuleCache::getCompiledMainModule(const std::string& user,
                                                        const std::string& func)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();
    const std::string key = getModuleKey(user, func, "");

    if (getCompiledModuleCount(key) == 0) {
        faabric::util::FullLock registryLock(registryMutex);
        if (compiledModuleMap.count(key) == 0) {
            IR::Module& module = getModuleFromMap(key);

            storage::FileLoader& functionLoader = storage::getFileLoader();
            faabric::Message msg = faabric::util::messageFactory(user, func);
            std::vector<uint8_t> objectFileBytes =
              functionLoader.loadFunctionObjectFile(msg);

            if (!objectFileBytes.empty()) {
                compiledModuleMap[key] =
                  Runtime::loadPrecompiledModule(module, objectFileBytes);
            } else {
                compiledModuleMap[key] = Runtime::compileModule(module);
            }
        }
    } else {
        logger->debug("Using cached compiled main module {}/{}", user, func);
    }

    return compiledModuleMap[key];
}

Runtime::ModuleRef IRModuleCache::getCompiledSharedModule(
  const std::string& user,
  const std::string& func,
  const std::string& path)
{
    std::string key = getModuleKey(user, func, path);
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();

    if (getCompiledModuleCount(key) == 0) {
        faabric::util::FullLock registryLock(registryMutex);
        if (compiledModuleMap.count(key) == 0) {
            logger->debug(
              "Loading compiled shared module {}/{} - {}", user, func, path);

            IR::Module& module = getModuleFromMap(key);

            storage::FileLoader& functionLoader = storage::getFileLoader();
            std::vector<uint8_t> objectBytes =
              functionLoader.loadSharedObjectObjectFile(path);
            compiledModuleMap[key] =
              Runtime::loadPrecompiledModule(module, objectBytes);
        }
    } else {
        logger->debug(
          "Using cached shared compiled module {}/{} - {}", user, func, path);
    }

    return compiledModuleMap[key];
}

IR::Module& IRModuleCache::getMainModule(const std::string& user,
                                         const std::string& func)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();
    const std::string key = getModuleKey(user, func, "");

    // Check if initialised
    if (getModuleCount(key) == 0) {
        faabric::util::FullLock registryLock(registryMutex);
        if (moduleMap.count(key) == 0) {
            logger->debug("Loading main module {}/{}", user, func);

            storage::FileLoader& functionLoader = storage::getFileLoader();

            faabric::Message msg = faabric::util::messageFactory(user, func);
            std::vector<uint8_t> wasmBytes =
              functionLoader.loadFunctionWasm(msg);

            IR::Module& module = getModuleFromMap(key);

            if (faabric::util::isWasm(wasmBytes)) {
                WASM::LoadError loadError;
                WASM::loadBinaryModule(
                  wasmBytes.data(), wasmBytes.size(), module, &loadError);
            } else {
                std::vector<WAST::Error> parseErrors;
                WAST::parseModule((const char*)wasmBytes.data(),
                                  wasmBytes.size(),
                                  module,
                                  parseErrors);
                WAST::reportParseErrors(
                  "wast_file", (const char*)wasmBytes.data(), parseErrors);
            }

            // Force maximum size
            module.memories.defs[0].type.size.max = (U64)MAX_MEMORY_PAGES;

            // Typescript modules don't seem to define a table
            if (!module.tables.defs.empty()) {
                module.tables.defs[0].type.size.max = (U64)MAX_TABLE_SIZE;
            }
        }
    } else {
        logger->debug("Using cached main module {}/{}", user, func);
    }

    return getModuleFromMap(key);
}

IR::Module& IRModuleCache::getSharedModule(const std::string& user,
                                           const std::string& func,
                                           const std::string& path)
{
    std::string key = getModuleKey(user, func, path);
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();

    // Check if initialised
    if (getModuleCount(key) == 0) {
        faabric::util::FullLock lock(registryMutex);
        if (moduleMap.count(key) == 0) {
            logger->debug("Loading shared module {}/{} - {}", user, func, path);

            storage::FileLoader& functionLoader = storage::getFileLoader();

            std::vector<uint8_t> wasmBytes =
              functionLoader.loadSharedObjectWasm(path);

            IR::Module& module = getModuleFromMap(key);

            WASM::LoadError loadError;
            WASM::loadBinaryModule(
              wasmBytes.data(), wasmBytes.size(), module, &loadError);

            // Check that the module isn't expecting to create any memories or
            // tables
            if (!module.tables.defs.empty()) {
                throw std::runtime_error(
                  "Dynamic module trying to define tables");
            }

            if (!module.memories.defs.empty()) {
                throw std::runtime_error(
                  "Dynamic module trying to define memories");
            }

            // TODO - better way to handle this? Modify WAVM?  To keep WAVM
            // happy, we have to force the incoming dynamic module to accept
            // the table from the main module. This modifies the shared
            // reference, therefore we also have to preserve the original
            // size and make available to callers.
            this->originalTableSizes[key] =
              module.tables.imports[0].type.size.min;

            const std::string mainKey = getModuleKey(user, func, "");
            IR::Module& mainModule = getModuleFromMap(mainKey);

            module.tables.imports[0].type.size.min =
              (U64)mainModule.tables.defs[0].type.size.min;
            module.tables.imports[0].type.size.max =
              (U64)mainModule.tables.defs[0].type.size.max;
        }
    } else {
        logger->debug(
          "Loading cached shared module {}/{} - {}", user, func, path);
    }

    return getModuleFromMap(key);
}

bool IRModuleCache::isModuleCached(const std::string& user,
                                   const std::string& func,
                                   const std::string& path)
{
    std::string key = getModuleKey(user, func, path);
    return getModuleCount(key) > 0;
}

bool IRModuleCache::isCompiledModuleCached(const std::string& user,
                                           const std::string& func,
                                           const std::string& path)
{
    std::string key = getModuleKey(user, func, path);
    return getCompiledModuleCount(key) > 0;
}

void IRModuleCache::clear()
{
    faabric::util::FullLock lock(registryMutex);

    auto logger = faabric::util::getLogger();
    logger->debug("Clearing IR cache");

    moduleMap.clear();
    compiledModuleMap.clear();
    originalTableSizes.clear();
}
}
