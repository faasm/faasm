#pragma once

#include <wasm/WasmModule.h>

namespace module_cache {
    class WasmModuleCache {
    public:
        wasm::WasmModule &getCachedModule(const message::Message &msg);

        void clear();

        size_t getTotalCachedModuleCount();
    private:
        std::shared_mutex mx;
        std::unordered_map<std::string, wasm::WasmModule> cachedModuleMap;

        std::string getCachedModuleKey(const message::Message &msg);

        std::string getBaseCachedModuleKey(const message::Message &msg);

        int getCachedModuleCount(const std::string &key);
    };

    WasmModuleCache &getWasmModuleCache();
}
