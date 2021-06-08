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

    WasmModule* previousModule = nullptr;
    faabric::Message* previousCall = nullptr;

    WasmExecutionContext(WasmModule* module, faabric::Message* call);

    ~WasmExecutionContext();
};

WasmModule* getExecutingModule();

faabric::Message* getExecutingCall();
}
