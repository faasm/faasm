#pragma once

#include <sgx/system.h>

#include <memory>
#include <unordered_map>

#include <wasm_export.h>

namespace sgx {
struct WamrModuleHandle
{
    wasm_module_t wasmModule;
    wasm_module_inst_t moduleInstance;
    uint8_t* wasmOpCodePtr = NULL;
    uint32_t wasmOpCodeSize = 0;
};

// Writes to the module store are protected by the locking mechanism outside
// the enclave. We may potentially have multiple readers at the same
// time.
class ModuleStore
{
  public:
    ModuleStore();

    std::shared_ptr<sgx::WamrModuleHandle> store(const char* key,
                                                 const void* ptr,
                                                 const uint32_t size);

    std::shared_ptr<sgx::WamrModuleHandle> get(const char* key);

    void clear();

  private:
    std::unordered_map<std::string, std::shared_ptr<sgx::WamrModuleHandle>>
      modules;
};
}
