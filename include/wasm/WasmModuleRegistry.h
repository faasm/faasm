#pragma once

#include "WasmModule.h"

#include <mutex>


using namespace WAVM;

namespace wasm {
    class WasmModuleRegistry {
    public:
        WasmModuleRegistry();

        bool moduleExists(const std::string &key);

        void registerModule(const std::string &key, const WasmModule &module);

        WasmModule &getModule(const std::string &key);
    private:
        std::mutex registryMutex;
        std::unordered_map<std::string, WasmModule> moduleMap;
    };

    WasmModuleRegistry &getWasmModuleRegistry();
}

