#pragma once

#include <wavm/WAVMWasmModule.h>

namespace module_cache {
    class WasmModuleCache {
    public:
        wasm::WAVMWasmModule &getCachedModule(const faabric::Message &msg);

        void clear();

        size_t getTotalCachedModuleCount();
    private:
        std::shared_mutex mx;
        std::unordered_map<std::string, wasm::WAVMWasmModule> cachedModuleMap;

        std::string getCachedModuleKey(const faabric::Message &msg);

        std::string getBaseCachedModuleKey(const faabric::Message &msg);

        int getCachedModuleCount(const std::string &key);
    };

    WasmModuleCache &getWasmModuleCache();
}
