#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

// Defined in wasi-libc/libc-bottom-half/sources/getcwd.c
#define WASI_LIBC_CWD "/"

#define FAKE_NAME "faasm"
#define FAKE_PASSWORD "foobar123"
#define FAKE_HOME "/home/faasm/"
#define FAKE_HOSTNAME "faasm"

#define FAKE_PID 23
#define FAKE_TID 66
#define FAKE_UID 1000
#define FAKE_GID 1000
#define FAKE_N_PROCESSORS 4

#define MAIN_MODULE_DYNLINK_HANDLE 999

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
