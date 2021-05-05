#pragma once

#include <wavm/WAVMWasmModule.h>

namespace wasm {
class WAVMModuleCache
{
  public:
    wasm::WAVMWasmModule& getCachedModule(const faabric::Message& msg);

    void clear();

    size_t getTotalCachedModuleCount();

  private:
    std::shared_mutex mx;
    std::unordered_map<std::string, wasm::WAVMWasmModule> cachedModuleMap;

    int getCachedModuleCount(const std::string& key);
};

WAVMModuleCache& getWAVMModuleCache();
}
