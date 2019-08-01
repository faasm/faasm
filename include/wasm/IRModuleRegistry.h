#pragma once

#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/IR/Module.h>

#include <mutex>

// Note that page size in wasm is 64kiB
// Note also that this initial memory must be big enough to include all data, stack and dynamic
// memory that the module will need.
#define ONE_MB_PAGES 16
#define ONE_GB_PAGES 1024 * ONE_MB_PAGES
#define MAX_MEMORY_PAGES ONE_GB_PAGES
#define MAX_TABLE_SIZE 500000

using namespace WAVM;

namespace wasm {
    class IRModuleRegistry {
    public:
        IR::Module &getModule(const std::string &user, const std::string &func, const std::string &path);

        Runtime::ModuleRef getCompiledModule(const std::string &user, const std::string &func,
                const std::string &path);

    private:
        std::mutex registryMutex;
        std::unordered_map<std::string, IR::Module> moduleMap;
        std::unordered_map<std::string, Runtime::ModuleRef> compiledModuleMap;

        IR::Module &getMainModule(const std::string &user, const std::string &func);

        IR::Module &getSharedModule(const std::string &user, const std::string &func, const std::string &path);

        Runtime::ModuleRef getCompiledMainModule(const std::string &user, const std::string &func);

        Runtime::ModuleRef getCompiledSharedModule(const std::string &user, const std::string &func,
                const std::string &path);
    };

    IRModuleRegistry &getIRModuleRegistry();
}
