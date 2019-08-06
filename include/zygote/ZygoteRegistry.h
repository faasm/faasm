#pragma once

#include <wasm/WasmModule.h>

namespace zygote {
    class ZygoteRegistry {
    public:
        wasm::WasmModule &getZygote(const message::Message &msg);
    private:
        std::mutex mx;
        std::unordered_map<std::string, wasm::WasmModule> zygoteMap;
    };

    ZygoteRegistry &getZygoteRegistry();
}
