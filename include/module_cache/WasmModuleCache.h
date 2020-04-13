#pragma once

#include <wasm/WAVMWasmModule.h>

namespace module_cache {
    class WasmModuleCache {
    public:
        wasm::WAVMWasmModule &getCachedModule(const message::Message &msg);

        void clear();

        size_t getTotalCachedModuleCount();
    private:
        std::shared_mutex mx;
        std::unordered_map<std::string, wasm::WAVMWasmModule> cachedModuleMap;

        std::string getCachedModuleKey(const message::Message &msg);

        std::string getBaseCachedModuleKey(const message::Message &msg);

        int getCachedModuleCount(const std::string &key);
    };

    WasmModuleCache &getWasmModuleCache();
}
