#include "IRModuleRegistry.h"

#include <util/locks.h>

#include <WAVM/IR/Module.h>
#include <WAVM/WASM/WASM.h>
#include <WAVM/IR/Types.h>
#include <WAVM/WASTParse/WASTParse.h>

#include <wasm/FunctionLoader.h>
#include <util/func.h>

using namespace WAVM;

namespace wasm {
    static std::mutex registryMutex;
    static std::unordered_map<std::string, IR::Module> moduleMap;
    static std::unordered_map<std::string, Runtime::ModuleRef> compiledModuleMap;

    IRModuleRegistry &getIRModuleRegistry() {
        static IRModuleRegistry r;
        return r;
    }

    std::string getModuleKey(const std::string &user, const std::string &func, const std::string &path) {
        std::string key = user + "_" + func + "_" + path;
        return key;
    }

    IR::Module &IRModuleRegistry::getModule(const std::string &user, const std::string &func,
                                            const std::string &sharedLibraryPath) {
        if (sharedLibraryPath.empty()) {
            return getMainModule(user, func);
        } else {
            return getSharedModule(user, func, sharedLibraryPath);
        }
    }

    Runtime::ModuleRef IRModuleRegistry::getCompiledModule(const std::string &user, const std::string &func,
                                                           const std::string &sharedLibraryPath) {
        if (sharedLibraryPath.empty()) {
            return getCompiledMainModule(user, func);
        } else {
            return getCompiledSharedModule(user, func, sharedLibraryPath);
        }
    }

    Runtime::ModuleRef IRModuleRegistry::getCompiledMainModule(const std::string &user, const std::string &func) {
        const std::string key = getModuleKey(user, func, "");

        if (compiledModuleMap.count(key) == 0) {
            util::UniqueLock registryLock(registryMutex);
            if (compiledModuleMap.count(key) == 0) {
                IR::Module &module = moduleMap[key];

                wasm::FunctionLoader &functionLoader = wasm::getFunctionLoader();

                message::Message msg = util::messageFactory(user, func);
                std::vector<uint8_t> objectFileBytes = functionLoader.loadFunctionObjectBytes(msg);

                if (!objectFileBytes.empty()) {
                    compiledModuleMap[key] = Runtime::loadPrecompiledModule(module, objectFileBytes);
                } else {
                    compiledModuleMap[key] = Runtime::compileModule(module);
                }
            }
        }

        return compiledModuleMap[key];
    }

    Runtime::ModuleRef IRModuleRegistry::getCompiledSharedModule(
            const std::string &user, const std::string &func,
            const std::string &path) {
        std::string key = getModuleKey(user, func, path);

        if (compiledModuleMap.count(key) == 0) {
            util::UniqueLock registryLock(registryMutex);
            if (compiledModuleMap.count(key) == 0) {
                IR::Module &module = moduleMap[key];

                wasm::FunctionLoader &functionLoader = wasm::getFunctionLoader();
                std::string objFilePath = path + SHARED_OBJ_EXT;

                std::vector<uint8_t> objectBytes = functionLoader.loadFileBytes(objFilePath);
                compiledModuleMap[key] = Runtime::loadPrecompiledModule(module, objectBytes);
            }
        }

        return compiledModuleMap[key];
    }

    IR::Module &IRModuleRegistry::getMainModule(const std::string &user, const std::string &func) {
        const std::string key = getModuleKey(user, func, "");

        // Check if initialised
        if (moduleMap.count(key) == 0) {
            // Get lock and check again
            util::UniqueLock registryLock(registryMutex);
            if (moduleMap.count(key) == 0) {
                wasm::FunctionLoader &functionLoader = wasm::getFunctionLoader();

                message::Message msg = util::messageFactory(user, func);
                std::vector<uint8_t> wasmBytes = functionLoader.loadFunctionBytes(msg);

                IR::Module &module = moduleMap[key];
                this->initialiseMainModule(module, wasmBytes);
            }
        }

        return moduleMap[key];
    }

    IR::Module &IRModuleRegistry::getSharedModule(
            const std::string &user, const std::string &func, const std::string &path
    ) {
        std::string key = getModuleKey(user, func, path);

        // Check if initialised
        if (moduleMap.count(key) == 0) {
            // Get lock and check again
            util::UniqueLock registryLock(registryMutex);
            if (moduleMap.count(key) == 0) {
                wasm::FunctionLoader &functionLoader = wasm::getFunctionLoader();

                IR::Module &mainModule = this->getMainModule(user, func);

                std::vector<uint8_t> wasmBytes = functionLoader.loadFileBytes(path);
                IR::Module &module = moduleMap[key];
                this->initialiseSharedModule(module, mainModule, wasmBytes);
            }
        }

        return moduleMap[key];
    }

    void IRModuleRegistry::initialiseMainModule(
            IR::Module &module,
            const std::vector<uint8_t> &wasmBytes
    ) {
        wasm::FunctionLoader &functionLoader = wasm::getFunctionLoader();

        if (functionLoader.isWasm(wasmBytes)) {
            WASM::loadBinaryModule(wasmBytes.data(), wasmBytes.size(), module);
        } else {
            std::vector<WAST::Error> parseErrors;
            WAST::parseModule((const char *) wasmBytes.data(), wasmBytes.size(), module, parseErrors);
            WAST::reportParseErrors("wast_file", parseErrors);
        }

        // Force memory sizes
        module.memories.defs[0].type.size.max = (U64) MAX_MEMORY_PAGES;

        // Note, we don't want to mess with the min table size here, just give it room to expand if need be
        module.tables.defs[0].type.size.max = (U64) MAX_TABLE_SIZE;
    }

    void IRModuleRegistry::initialiseSharedModule(
            IR::Module &module,
            IR::Module &mainModule,
            const std::vector<uint8_t> &wasmBytes
    ) {
        WASM::loadBinaryModule(wasmBytes.data(), wasmBytes.size(), module);

        // Check that the module isn't expecting to create any memories or tables
        if (!module.tables.defs.empty()) {
            throw std::runtime_error("Dynamic module trying to define tables");
        }

        if (!module.memories.defs.empty()) {
            throw std::runtime_error("Dynamic module trying to define memories");
        }

        // Now force the incoming dynamic module to accept the table from the main module
        module.tables.imports[0].type.size.min = (U64) mainModule.tables.defs[0].type.size.min;
        module.tables.imports[0].type.size.max = (U64) mainModule.tables.defs[0].type.size.max;
    }
}