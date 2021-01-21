#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace wasm {
class WasmEnvironment
{
  public:
    WasmEnvironment();

    void addEnv(const std::string& key, const std::string& value);

    uint32_t getEnvCount();

    uint32_t getEnvBufferSize();

    std::vector<std::string> getVars();

    std::string getEnv(const std::string& key);

    void printDebugInfo();

  private:
    std::unordered_map<std::string, std::string> vars;
};
}
