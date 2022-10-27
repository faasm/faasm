#include <faabric/util/files.h>
#include <faabric/util/func.h>
#include <faabric/util/locks.h>
#include <faabric/util/logging.h>
#include <storage/FileLoader.h>
#include <wasm/WasmCommon.h>
#include <wavm/IRModuleCache.h>

#include <WAVM/IR/Module.h>
#include <WAVM/IR/Types.h>
#include <WAVM/WASM/WASM.h>
#include <WAVM/WASTParse/WASTParse.h>

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
    faabric::util::SharedLock lock(mx);
    return moduleMap.count(key);
}

int IRModuleCache::getCompiledModuleCount(const std::string& key)
{
    faabric::util::SharedLock lock(mx);
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

size_t IRModuleCache::getSharedModuleDataSize(const std::string& user,
                                              const std::string& func,
                                              const std::string& path)
{
    IR::Module& irModule = IRModuleCache::getModule(user, func, path);
    size_t dataSize = 0;
    for (auto ds : irModule.dataSegments) {
        dataSize += ds.data->size();
    }

    return dataSize;
}

Runtime::ModuleRef IRModuleCache::getCompiledMainModule(const std::string& user,
                                                        const std::string& func)
{

    const std::string key = getModuleKey(user, func, "");

    if (getCompiledModuleCount(key) == 0) {
        faabric::util::FullLock registryLock(mx);
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
        SPDLOG_DEBUG("Using cached compiled main module {}/{}", user, func);
    }

    return compiledModuleMap[key];
}

Runtime::ModuleRef IRModuleCache::getCompiledSharedModule(
  const std::string& user,
  const std::string& func,
  const std::string& path)
{
    std::string key = getModuleKey(user, func, path);

    if (getCompiledModuleCount(key) == 0) {
        faabric::util::FullLock registryLock(mx);
        if (compiledModuleMap.count(key) == 0) {
            SPDLOG_DEBUG(
              "Loading compiled shared module {}/{} - {}", user, func, path);

            IR::Module& module = getModuleFromMap(key);

            storage::FileLoader& functionLoader = storage::getFileLoader();
            std::vector<uint8_t> objectBytes =
              functionLoader.loadSharedObjectObjectFile(path);
            compiledModuleMap[key] =
              Runtime::loadPrecompiledModule(module, objectBytes);
        }
    } else {
        SPDLOG_DEBUG(
          "Using cached shared compiled module {}/{} - {}", user, func, path);
    }

    {
        faabric::util::SharedLock lock(mx);
        return compiledModuleMap[key];
    }
}

IR::Module& IRModuleCache::getMainModule(const std::string& user,
                                         const std::string& func)
{

    const std::string key = getModuleKey(user, func, "");

    // Check if initialised
    if (getModuleCount(key) == 0) {
        faabric::util::FullLock registryLock(mx);
        if (moduleMap.count(key) == 0) {
            SPDLOG_DEBUG("Loading main module {}/{}", user, func);

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
            if (module.memories.defs.empty()) {
                SPDLOG_ERROR("WASM module ({}) does not define any memories",
                             key);
                throw std::runtime_error(
                  "WASM module does not define any memories");
            }
            module.memories.defs[0].type.size.max = (U64)MAX_WASM_MEMORY_PAGES;

            // Typescript modules don't seem to define a table
            if (!module.tables.defs.empty()) {
                module.tables.defs[0].type.size.max = (U64)MAX_TABLE_SIZE;
            }
        }
    } else {
        SPDLOG_DEBUG("Using cached main module {}/{}", user, func);
    }

    {
        faabric::util::SharedLock lock(mx);
        return getModuleFromMap(key);
    }
}

IR::Module& IRModuleCache::getSharedModule(const std::string& user,
                                           const std::string& func,
                                           const std::string& path)
{
    std::string key = getModuleKey(user, func, path);

    // Check if initialised
    if (getModuleCount(key) == 0) {
        faabric::util::FullLock lock(mx);
        if (moduleMap.count(key) == 0) {
            SPDLOG_DEBUG("Loading shared module {}/{} - {}", user, func, path);

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
        SPDLOG_DEBUG(
          "Loading cached shared module {}/{} - {}", user, func, path);
    }

    {
        faabric::util::SharedLock lock(mx);
        return getModuleFromMap(key);
    }
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
    faabric::util::FullLock lock(mx);

    SPDLOG_DEBUG("Clearing IR cache");

    moduleMap.clear();
    compiledModuleMap.clear();
    originalTableSizes.clear();
}
}
