#include "IRModuleRegistry.h"

#include <util/func.h>
#include <util/locks.h>
#include <util/logging.h>

#include <WAVM/IR/Module.h>
#include <WAVM/WASM/WASM.h>
#include <WAVM/IR/Types.h>
#include <WAVM/IR/Module.h>
#include <WAVM/WASTParse/WASTParse.h>

#include <wasm/FunctionLoader.h>

using namespace WAVM;

namespace wasm {
    static std::mutex registryMutex;
    static std::unordered_map<std::string, IR::Module> moduleMap;
    static std::unordered_map<std::string, Runtime::ModuleRef> compiledModuleMap;

    IRModuleRegistry &getIRModuleRegistry() {
        static IRModuleRegistry r;
        return r;
    }

    IR::Module &IRModuleRegistry::getMainModule(const std::string &user, const std::string &func) {
        message::Message msg;
        msg.set_user(user);
        msg.set_function(func);

        return this->getMainModule(msg);
    }

    Runtime::ModuleRef &IRModuleRegistry::getCompiledModule(const std::string &user, const std::string &func) {
        message::Message msg;
        msg.set_user(user);
        msg.set_function(func);

        return this->getCompiledModule(msg);
    }

    Runtime::ModuleRef &IRModuleRegistry::getCompiledModule(const message::Message &msg) {
        const std::string key = util::funcToString(msg);
        return compiledModuleMap[key];
    }

    Runtime::ModuleRef &IRModuleRegistry::getCompiledSharedModule(const std::string &path) {
        return compiledModuleMap[path];
    }

    IR::Module &IRModuleRegistry::getMainModule(const message::Message &msg) {
        // TODO - thread safe?
        const std::string key = util::funcToString(msg);
        IR::Module &module = moduleMap[key];

        // Check if initialised
        if (module.exports.empty()) {
            // Get lock and check again
            util::UniqueLock registryLock(registryMutex);
            if (module.exports.empty()) {
                // Do initialisation
                wasm::FunctionLoader &functionLoader = wasm::getFunctionLoader();
                std::vector<uint8_t> wasmBytes = functionLoader.loadFunctionBytes(msg);
                std::vector<uint8_t> objectFileBytes = functionLoader.loadFunctionObjectBytes(msg);

                Runtime::ModuleRef &compiledModule = compiledModuleMap[key];
                
                this->initialiseIRModule(module, compiledModule, wasmBytes, objectFileBytes, true);
            }
        }

        return module;
    }

    IR::Module &IRModuleRegistry::getSharedModule(const std::string &path) {
        IR::Module &module = moduleMap[path];

        // Check if initialised
        if (module.exports.empty()) {
            // Get lock and check again
            util::UniqueLock registryLock(registryMutex);
            if (module.exports.empty()) {
                // Get path to where machine code should be
                wasm::FunctionLoader &functionLoader = wasm::getFunctionLoader();
                std::string objFilePath = path + SHARED_OBJ_EXT;
                std::vector<uint8_t> wasmBytes = functionLoader.loadFunctionBytes(path);
                std::vector<uint8_t> objectBytes = functionLoader.loadFunctionObjectBytes(objFilePath);

                Runtime::ModuleRef &compiledModule = compiledModuleMap[path];

                this->initialiseIRModule(module, compiledModule, wasmBytes, objectBytes, false);
            }
        }

        return module;
    }

    void IRModuleRegistry::initialiseIRModule(
            IR::Module &module,
            Runtime::ModuleRef &compiledModule,
            const std::vector<uint8_t> &wasmBytes,
            const std::vector<uint8_t> &objBytes,
            bool isMainModule
    ) {
        wasm::FunctionLoader &functionLoader = wasm::getFunctionLoader();

        if (functionLoader.isWasm(wasmBytes)) {
            WASM::loadBinaryModule(wasmBytes.data(), wasmBytes.size(), module);
        } else {
            std::vector<WAST::Error> parseErrors;
            WAST::parseModule((const char *) wasmBytes.data(), wasmBytes.size(), module, parseErrors);
            WAST::reportParseErrors("wast_file", parseErrors);
        }

        if (isMainModule) {
            // Force memory sizes
            module.memories.defs[0].type.size.max = (U64) MAX_MEMORY_PAGES;

            // Note, we don't want to mess with the min table size here, just give it room to expand if need be
            module.tables.defs[0].type.size.max = (U64) MAX_TABLE_SIZE;
        }

        if (!objBytes.empty()) {
            compiledModule = Runtime::loadPrecompiledModule(module, objBytes);
        } else {
            compiledModule = Runtime::compileModule(module);
        }
    }
}