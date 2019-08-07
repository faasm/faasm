#include "IRModuleRegistry.h"

#include <util/locks.h>
#include <util/logging.h>

#include <WAVM/IR/Module.h>
#include <WAVM/WASM/WASM.h>
#include <WAVM/IR/Types.h>
#include <WAVM/WASTParse/WASTParse.h>

#include <storage/FunctionLoader.h>
#include <util/func.h>

using namespace WAVM;

namespace wasm {
    IRModuleRegistry &getIRModuleRegistry() {
        static IRModuleRegistry r;
        return r;
    }

    std::string getModuleKey(const std::string &user, const std::string &func, const std::string &path) {
        std::string key = user + "_" + func + "_" + path;
        return key;
    }

    int IRModuleRegistry::getModuleCount(const std::string &key) {
        util::SharedLock lock(registryMutex);
        return moduleMap.count(key);
    }

    int IRModuleRegistry::getCompiledModuleCount(const std::string &key) {
        util::SharedLock lock(registryMutex);
        return compiledModuleMap.count(key);
    }

    IR::Module &IRModuleRegistry::getModule(const std::string &user, const std::string &func, const std::string &path) {
        /*
         * Note that shared modules are currently only shared in memory across instances of the *same* function.
         * If two different functions both load shared module A, it will be loaded into memory twice.
         *
         * This is currently only to do with needing to modify the table definition to fit with the importing main
         * module, something which can probably be fixed.
         *
         * TODO - implement this proper sharing of modules
         */

        if (path.empty()) {
            return this->getMainModule(user, func);
        } else {
            return this->getSharedModule(user, func, path);
        }
    }

    Runtime::ModuleRef IRModuleRegistry::getCompiledModule(const std::string &user, const std::string &func,
                                                           const std::string &path) {
        if (path.empty()) {
            return this->getCompiledMainModule(user, func);
        } else {
            return this->getCompiledSharedModule(user, func, path);
        }
    }

    U64 IRModuleRegistry::getSharedModuleTableSize(const std::string &user, const std::string &func,
                                                   const std::string &path) {
        const std::string key = getModuleKey(user, func, path);
        return originalTableSizes[key];
    }

    Runtime::ModuleRef IRModuleRegistry::getCompiledMainModule(const std::string &user, const std::string &func) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        const std::string key = getModuleKey(user, func, "");

        if (getCompiledModuleCount(key) == 0) {
            util::FullLock registryLock(registryMutex);
            if (compiledModuleMap.count(key) == 0) {
                logger->debug("Loading compiled main module {}", key);

                IR::Module &module = moduleMap[key];
                storage::FunctionLoader &functionLoader = storage::getFunctionLoader();

                message::Message msg = util::messageFactory(user, func);
                std::vector<uint8_t> objectFileBytes = functionLoader.loadFunctionObjectBytes(msg);

                if (!objectFileBytes.empty()) {
                    compiledModuleMap[key] = Runtime::loadPrecompiledModule(module, objectFileBytes);
                } else {
                    compiledModuleMap[key] = Runtime::compileModule(module);
                }
            }
        } else {
            logger->debug("Using cached compiled main module {}", key);
        }

        return compiledModuleMap[key];
    }

    Runtime::ModuleRef IRModuleRegistry::getCompiledSharedModule(const std::string &user, const std::string &func,
                                                                 const std::string &path) {
        std::string key = getModuleKey(user, func, path);
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        if (getCompiledModuleCount(key) == 0) {
            util::FullLock registryLock(registryMutex);
            if (compiledModuleMap.count(key) == 0) {
                logger->debug("Loading compiled shared module {}", key);

                IR::Module &module = moduleMap[key];

                storage::FunctionLoader &functionLoader = storage::getFunctionLoader();
                std::string objFilePath = path + SHARED_OBJ_EXT;

                std::vector<uint8_t> objectBytes = functionLoader.loadFileBytes(objFilePath);
                compiledModuleMap[key] = Runtime::loadPrecompiledModule(module, objectBytes);
            }
        } else {
            logger->debug("Using cached shared compiled module {}", key);
        }

        return compiledModuleMap[key];
    }

    IR::Module &IRModuleRegistry::getMainModule(const std::string &user, const std::string &func) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        const std::string key = getModuleKey(user, func, "");

        // Check if initialised
        if (getModuleCount(key) == 0) {
            util::FullLock registryLock(registryMutex);
            if (moduleMap.count(key) == 0) {
                logger->debug("Loading main module {}", key);

                storage::FunctionLoader &functionLoader = storage::getFunctionLoader();

                message::Message msg = util::messageFactory(user, func);
                std::vector<uint8_t> wasmBytes = functionLoader.loadFunctionBytes(msg);

                IR::Module &module = moduleMap[key];

                if (functionLoader.isWasm(wasmBytes)) {
                    WASM::loadBinaryModule(wasmBytes.data(), wasmBytes.size(), module);
                } else {
                    std::vector<WAST::Error> parseErrors;
                    WAST::parseModule((const char *) wasmBytes.data(), wasmBytes.size(), module, parseErrors);
                    WAST::reportParseErrors("wast_file", parseErrors);
                }

                // Force maximum memory and table size
                module.memories.defs[0].type.size.max = (U64) MAX_MEMORY_PAGES;
                module.tables.defs[0].type.size.max = (U64) MAX_TABLE_SIZE;
            }
        } else {
            logger->debug("Using cached main module {}", key);
        }

        return moduleMap[key];
    }

    IR::Module &IRModuleRegistry::getSharedModule(const std::string &user, const std::string &func,
                                                  const std::string &path) {
        std::string key = getModuleKey(user, func, path);
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Check if initialised
        if (getModuleCount(key) == 0) {
            util::FullLock lock(registryMutex);
            if (moduleMap.count(key) == 0) {
                logger->debug("Loading shared module {}", key);

                storage::FunctionLoader &functionLoader = storage::getFunctionLoader();

                std::vector<uint8_t> wasmBytes = functionLoader.loadFileBytes(path);

                IR::Module &module = moduleMap[key];
                WASM::loadBinaryModule(wasmBytes.data(), wasmBytes.size(), module);

                // Check that the module isn't expecting to create any memories or tables
                if (!module.tables.defs.empty()) {
                    throw std::runtime_error("Dynamic module trying to define tables");
                }

                if (!module.memories.defs.empty()) {
                    throw std::runtime_error("Dynamic module trying to define memories");
                }

                // TODO - better way to handle this? Modify WAVM?
                // To keep WAVM happy, we have to force the incoming dynamic module to accept the table from the
                // main module. This modifies the shared reference, therefore we also have to preserve the original
                // size and make available to callers.
                this->originalTableSizes[key] = module.tables.imports[0].type.size.min;

                IR::Module &mainModule = this->getMainModule(user, func);
                module.tables.imports[0].type.size.min = (U64) mainModule.tables.defs[0].type.size.min;
                module.tables.imports[0].type.size.max = (U64) mainModule.tables.defs[0].type.size.max;
            }
        } else {
            logger->debug("Loading cached shared module {}", key);
        }

        return moduleMap[key];
    }
}