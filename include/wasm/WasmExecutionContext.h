#pragma once

#include <wasm/WasmModule.h>

namespace wasm {

class WasmExecutionContext;

WasmExecutionContext* getCurrentWasmExecutionContext();

class WasmExecutionContext
{
  public:
    WasmModule* executingModule = nullptr;

    WasmExecutionContext(WasmModule* module);

    ~WasmExecutionContext();

    WasmExecutionContext(const WasmExecutionContext&) = delete;
    WasmExecutionContext& operator=(const WasmExecutionContext&) = delete;
};

WasmModule* getExecutingModule();
}
