#pragma once

#include <wasm/WasmModule.h>

#include <WAVM/Runtime/Runtime.h>

namespace wasm {
class LoadedDynamicModule
{
  public:
    std::string path;

    uint32_t memoryBottom;
    uint32_t memoryTop;

    uint32_t stackTop;
    uint32_t stackPointer;
    uint32_t stackSize;

    uint32_t heapBottom;

    uint32_t tableBottom;
    uint32_t tableTop;

    WAVM::Runtime::GCPointer<WAVM::Runtime::Instance> ptr;

    bool validate();

    void printDebugInfo();

    void log();
};
}
