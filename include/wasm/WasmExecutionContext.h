#pragma once

#include <wasm/WasmModule.h>

namespace wasm {

class WasmExecutionContext;

WasmExecutionContext* getCurrentWasmExecutionContext();

class WasmExecutionContext
{
  public:
    WasmModule* executingModule = nullptr;
    faabric::Message* executingCall = nullptr;

    WasmExecutionContext(WasmModule* module, faabric::Message* call);

    ~WasmExecutionContext();

    WasmExecutionContext(const WasmExecutionContext&) = delete;
    WasmExecutionContext& operator=(const WasmExecutionContext&) = delete;
};

WasmModule* getExecutingModule();

faabric::Message* getExecutingCall();
}
