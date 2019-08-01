#include "WasmModuleRegistry.h"

#include <util/locks.h>

namespace wasm {
    WasmModuleRegistry &getWasmModuleRegistry() {
        static WasmModuleRegistry w;
        return w;
    }

    WasmModuleRegistry::WasmModuleRegistry() = default;

    bool WasmModuleRegistry::moduleExists(const std::string &key) {
        util::UniqueLock lock(registryMutex);

        return moduleMap.count(key) == 0;
    }

    void WasmModuleRegistry::registerModule(const std::string &key, const WasmModule &module) {
        util::UniqueLock lock(registryMutex);

        // Be careful that we create a new instance here
        moduleMap.emplace(key, module);
    }

    WasmModule &WasmModuleRegistry::getModule(const std::string &key) {
        util::UniqueLock lock(registryMutex);

        if(!moduleExists(key)) {
            throw std::runtime_error("Key does not exist: " + key);
        }

        return moduleMap[key];
    }
}