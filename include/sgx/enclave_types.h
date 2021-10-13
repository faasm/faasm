#pragma once

#include <wasm_export.h>

struct WamrModuleHandle
{
    wasm_module_t wasmModule;
    wasm_module_inst_t moduleInstance;
    uint8_t* wasmOpCode;
};
