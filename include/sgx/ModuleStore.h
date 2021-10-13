#pragma once

#include <vector>

#include <wasm_export.h>

namespace sgx {

struct WamrModuleHandle
{
    wasm_module_t wasmModule;
    wasm_module_inst_t moduleInstance;
    uint8_t* wasmOpCodePtr;
    uint32_t wasmOpCodeSize;
    bool isSet = false;
};

class ModuleStore
{
  public:
    ModuleStore(int size);

    uint32_t store(const void* ptr, const uint32_t size);

  private:
    std::vector<sgx::WamrModuleHandle> modules;

    uint32_t storeSize;

    uint32_t getFreeSlot();
};
}
