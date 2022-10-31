#pragma once

#include <WAVM/IR/Module.h>
#include <WAVM/Runtime/Intrinsics.h>

#include <faabric/util/config.h>
#include <shared_mutex>

using namespace WAVM;

namespace wasm {
class IRModuleCache
{
  public:
    IRModuleCache();

    IR::Module& getModule(const std::string& user,
                          const std::string& func,
                          const std::string& path);

    Runtime::ModuleRef getCompiledModule(const std::string& user,
                                         const std::string& func,
                                         const std::string& path);

    U64 getSharedModuleTableSize(const std::string& user,
                                 const std::string& func,
                                 const std::string& path);

    size_t getSharedModuleDataSize(const std::string& user,
                                   const std::string& func,
                                   const std::string& path);

    bool isModuleCached(const std::string& user,
                        const std::string& func,
                        const std::string& path);

    bool isCompiledModuleCached(const std::string& user,
                                const std::string& func,
                                const std::string& path);

    void clear();

  private:
    std::shared_mutex mx;
    std::unordered_map<std::string, IR::Module> moduleMap;
    std::unordered_map<std::string, Runtime::ModuleRef> compiledModuleMap;
    std::unordered_map<std::string, int> originalTableSizes;

    faabric::util::SystemConfig& conf;

    int getModuleCount(const std::string& key);

    int getCompiledModuleCount(const std::string& key);

    IR::Module& getMainModule(const std::string& user, const std::string& func);

    IR::Module& getSharedModule(const std::string& user,
                                const std::string& func,
                                const std::string& path);

    Runtime::ModuleRef getCompiledMainModule(const std::string& user,
                                             const std::string& func);

    Runtime::ModuleRef getCompiledSharedModule(const std::string& user,
                                               const std::string& func,
                                               const std::string& path);

    IR::Module& getModuleFromMap(const std::string& key);
};

IRModuleCache& getIRModuleCache();
}
