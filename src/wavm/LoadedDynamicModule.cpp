#include "syscalls.h"

#include <wavm/LoadedDynamicModule.h>

#include <Runtime/RuntimePrivate.h>
#include <faabric/util/logging.h>

using namespace WAVM;

namespace wasm {

int32_t getModuleStackPointer(Runtime::Instance* module,
                              Runtime::Context* context)
{
    // Note: we rely on the stack pointer being the first global.
    // It's usually the only mutable global, so we can use this as a sense check
    Runtime::Global* stackPtrGlobal = module->globals[0];

    if (!stackPtrGlobal->type.isMutable) {
        auto logger = faabric::util::getLogger();
        logger->error("Expected stack pointer (first global) to be mutable.");
        throw std::runtime_error("Stack pointer not mutable");
    }

    const IR::Value& value = Runtime::getGlobalValue(context, stackPtrGlobal);

    return value.i32;
}

bool LoadedDynamicModule::validate()
{
    // The checks in this function are really assertions, as they should only go
    // wrong if the logic in the loading module is broken.
    auto logger = faabric::util::getLogger();

    if (memoryTop <= memoryBottom) {
        return false;
    }

    if (stackTop != memoryBottom + stackSize) {
        return false;
    }

    if (dataTop != heapBottom) {
        return false;
    }

    int32_t dataSize = dataTop - dataBottom;

    if (stackTop + dataSize != heapBottom) {
        return false;
    }

    if (stackPointer != stackTop - 1) {
        return false;
    }

    if (stackSize <= 0) {
        return false;
    }

    if (tableTop < tableBottom) {
        return false;
    }

    if (ptr == nullptr) {
        return false;
    }

    int32_t memSize = memoryTop - memoryBottom;
    int32_t heapSize = memoryTop - heapBottom;
    if (memSize <= heapSize) {
        return false;
    }

    return true;
}

void LoadedDynamicModule::printDebugInfo(Runtime::Context* context)
{
    printf("---- Dynamic module ----\n");

    size_t stackSizeBytes = stackTop - memoryBottom;
    size_t heapSizeBytes = memoryTop - stackTop;
    size_t dataSizeBytes = dataTop - dataBottom;

    float stackSizeMb = ((float)stackSizeBytes) / (1024 * 1024);
    float dataSizeMb = ((float)dataSizeBytes) / (1024 * 1024);
    float heapSizeMb = ((float)heapSizeBytes) / (1024 * 1024);

    int32_t actualStackPointer = getModuleStackPointer(ptr, context);

    printf("Path        %s\n", path.c_str());
    printf("Stack size  %10.3f MiB\n", stackSizeMb);
    printf("Data size   %10.3f MiB\n", dataSizeMb);
    printf("Heap size   %10.3f MiB\n", heapSizeMb);
    printf(
      "Stack ptr   %10i (actual %10i)\n", stackPointer, actualStackPointer);
    printf("Stack       %10u -> %10u\n", memoryBottom, stackTop);
    printf("Data        %10u -> %10u\n", dataBottom, dataTop);
    printf("Heap        %10u -> %10u\n", heapBottom, memoryTop);
    printf("Table       %10u -> %10u\n", tableBottom, tableTop);
}

void LoadedDynamicModule::log()
{
    auto logger = faabric::util::getLogger();
    logger->debug("New dynamic module {}: mem={}-{}, stack={}-{}, data={}-{}, "
                  "heap={}-{}, table={}-{}",
                  path,
                  memoryBottom,
                  memoryTop,
                  memoryBottom,
                  stackTop,
                  dataBottom,
                  dataTop,
                  heapBottom,
                  memoryTop,
                  tableBottom,
                  tableTop);
}
}
