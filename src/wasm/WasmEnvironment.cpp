#include "WasmEnvironment.h"

#include <cstdlib>
#include <stdexcept>
#include <string>

namespace wasm {
WasmEnvironment::WasmEnvironment()
{
    // We have to be careful to match some of these values to the underlying
    // system, otherwise there may be conflicts when performing I/O

    // Locale
    // std::string encoding = addFromSystemEnv("LC_CTYPE", "en_GB.UTF-8");
    // addFromSystemEnv("LC_NAME", "en_GB.UTF-8");
    // addFromSystemEnv("LANG", "en_GB.UTF-8");

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

    for (auto p : vars) {
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

std::string WasmEnvironment::addFromSystemEnv(const std::string& key,
                                              const std::string& fallback)
{
    const char* actual = std::getenv(key.c_str());

    std::string value;
    if (actual == nullptr) {
        value = fallback;
    } else {
        value = actual;
    }

    vars[key] = value;

    return value;
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
    for (auto p : vars) {
        printf("%s=%s\n", p.first.c_str(), p.second.c_str());
    }
    printf("--------------------------\n");
}
}
