#pragma once

#include <string>
#include <vector>

namespace wasm {
    class WasmEnvironment {
    public:
        WasmEnvironment();

        void addEnv(const std::string &val);

        uint32_t getEnvCount();
        uint32_t getEnvBufferSize();

        const std::vector<std::string> &getVars();
    private:
        std::vector<std::string> vars;
    };
}