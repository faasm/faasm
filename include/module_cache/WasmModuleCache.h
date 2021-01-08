#pragma once

#include <wavm/WAVMWasmModule.h>

namespace module_cache {
class WasmModuleCache
{
  public:
    wasm::WAVMWasmModule& getCachedModule(const faabric::Message& msg);

    void clear();

    size_t getTotalCachedModuleCount();

    std::string getModuleHash(const faabric::Message& msg);

    void setModuleHash(const faabric::Message& msg,
                       const std::vector<uint8_t> &wasmBytes);

  private:
    std::shared_mutex mx;
    std::unordered_map<std::string, wasm::WAVMWasmModule> cachedModuleMap;

    std::unordered_map<std::string, std::string> moduleHashMap;

    std::string getCachedModuleKey(const faabric::Message& msg);

    std::string getBaseCachedModuleKey(const faabric::Message& msg);

    int getCachedModuleCount(const std::string& key);
};

WasmModuleCache& getWasmModuleCache();
}
