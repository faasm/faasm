#pragma once

#include <wasm/WasmModule.h>

#include <WAVM/Runtime/Runtime.h>

namespace wasm {
int32_t getModuleStackPointer(WAVM::Runtime::Instance* module,
                              WAVM::Runtime::Context* context);

class LoadedDynamicModule
{
  public:
    std::string path;

    uint32_t memoryBottom = 0;
    uint32_t memoryTop = 0;

    uint32_t stackTop = 0;
    int32_t stackPointer = 0;
    uint32_t stackSize = 0;

    uint32_t dataBottom = 0;
    uint32_t dataTop = 0;

    uint32_t heapBottom = 0;

    uint32_t tableBottom = 0;
    uint32_t tableTop = 0;

    WAVM::Runtime::GCPointer<WAVM::Runtime::Instance> ptr = nullptr;

    bool validate();

    void printDebugInfo(WAVM::Runtime::Context* context);

    void log();
};
}
