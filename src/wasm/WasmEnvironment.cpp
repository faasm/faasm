#include "WasmEnvironment.h"

namespace wasm {
    WasmEnvironment::WasmEnvironment() {
        vars = {
                "PYTHONHOME=/lib",
                "PYTHON_PATH=/lib",
                "PYTHONHASHSEED=0",
                "PYTHONNOUSERSITE=on",
                "LC_CTYPE=en_GB.UTF-8",
                "PYTHONWASM=1",
                "OPENBLAS_NUM_THREADS=1",
                "MKL_NUM_THREADS=1",
        };
    }

    const std::vector<std::string> &WasmEnvironment::getVars() {
        return vars;
    }

    void WasmEnvironment::addEnv(const std::string &val) {
        vars.emplace_back(val);
    }

    uint32_t WasmEnvironment::getEnvCount() {
        return vars.size();
    }

    uint32_t WasmEnvironment::getEnvBufferSize() {
        uint32_t bufferSize = 0;
        for (const auto v : vars) {
            bufferSize += v.size() + 1;
        }

        return bufferSize;
    }
}