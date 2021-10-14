#pragma once

#include <memory>
#include <vector>

#include <wasm_export.h>

#define MAX_SGX_STORE_SIZE 32
#define SGX_MODULE_STORE_ERROR (MAX_SGX_STORE_SIZE + 1)

namespace sgx {
struct WamrModuleHandle
{
    wasm_module_t wasmModule;
    wasm_module_inst_t moduleInstance;
    uint8_t* wasmOpCodePtr = NULL;
    uint32_t wasmOpCodeSize = 0;
    bool isSet = false;
};

// This object stores the loaded modules inside the enclave. Modules may be
// repeated if different threads accessing the enclave want to use the same
// module. This is to avoid in-enclave concurrency control.
// Writes to the module store are protected by the locking mechanism outside
// the enclave. We may potentially have multiple readers at the same
// time, but never of the same
class ModuleStore
{
  public:
    ModuleStore(size_t size);

    // Store a module, set the slot value, and return a pointer to it
    std::shared_ptr<sgx::WamrModuleHandle> store(const void* ptr,
                                                 const uint32_t size,
                                                 uint32_t* slot);

    std::shared_ptr<sgx::WamrModuleHandle> get(const uint32_t slot);

    bool clear(const uint32_t slot);

  private:
    std::vector<std::shared_ptr<sgx::WamrModuleHandle>> modules;

    uint32_t storeSize;

    uint32_t getFreeSlot();
};
}
