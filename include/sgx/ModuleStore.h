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

// This object stores the loaded modules inside the enclave. Modules may be
// repeated if different threads accessing the enclave want to use the same
// module. This is to avoid in-enclave concurrency control.
// Writes to the module store are protected by the locking mechanism outside
// the enclave. We may potentially have multiple readers at the same
// time, but never of the same store entry.
class ModuleStore
{
  public:
    ModuleStore();

    std::shared_ptr<sgx::WamrModuleHandle> store(const char* key,
                                                 const void* ptr,
                                                 const uint32_t size);

    std::shared_ptr<sgx::WamrModuleHandle> get(const char* key);

    bool clear(const char* key);

  private:
    std::unordered_map<std::string, std::shared_ptr<sgx::WamrModuleHandle>>
      modules;
};
}
