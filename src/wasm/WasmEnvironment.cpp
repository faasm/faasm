#include "WasmEnvironment.h"

#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <string>

namespace wasm {
WasmEnvironment::WasmEnvironment()
{
    // Threading
    vars["MKL_NUM_THREADS"] = "1";
    vars["OPENBLAS_NUM_THREADS"] = "1";

    // Python hard-coded
    vars["PYTHON_PATH"] = "/";
    vars["PYTHONHOME"] = "/";
    vars["PYTHONHASHSEED"] = "0";
    vars["PYTHONNOUSERSITE"] = "on";
    vars["PYTHONWASM"] = "1";
}

std::vector<std::string> WasmEnvironment::getVars()
{
    // Format of this is to concatenate all the values
    std::vector<std::string> result;

    for (auto& p : vars) {
        result.emplace_back(p.first + "=" + p.second);
    }

    return result;
}

std::string WasmEnvironment::getEnv(const std::string& key)
{
    return vars[key];
}

void WasmEnvironment::addEnv(const std::string& key, const std::string& value)
{
    vars[key] = value;
}

uint32_t WasmEnvironment::getEnvCount()
{
    return vars.size();
}

uint32_t WasmEnvironment::getEnvBufferSize()
{
    uint32_t bufferSize = 0;
    for (const auto& v : vars) {
        // Format is {first}={second}{terminator}
        bufferSize += v.first.size() + v.second.size() + 2;
    }

    return bufferSize;
}

void WasmEnvironment::printDebugInfo()
{
    printf("---- Wasm environment ----\n");
    for (auto& p : vars) {
        printf("%s=%s\n", p.first.c_str(), p.second.c_str());
    }
    printf("--------------------------\n");
}
}
