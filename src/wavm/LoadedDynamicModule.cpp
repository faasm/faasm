#include "syscalls.h"

#include <wavm/LoadedDynamicModule.h>

#include <WAVM/Runtime/Runtime.h>
#include <faabric/util/logging.h>

using namespace WAVM;

namespace wasm {
bool LoadedDynamicModule::validate()
{
    auto logger = faabric::util::getLogger();

    if (memoryTop <= memoryBottom) {
        return false;
    }

    if (stackTop != memoryBottom + stackSize) {
        return false;
    }

    if (stackTop != heapBottom) {
        return false;
    }

    if (stackPointer != stackTop - 1) {
        return false;
    }

    if (stackSize <= 0) {
        return false;
    }

    if (tableTop <= tableBottom) {
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

void LoadedDynamicModule::print()
{
    printf("Dynamic module %s\n", path.c_str());

    printf("Stack      %10u -> %10u\n", memoryBottom, stackTop);
    printf("Heap       %10u -> %10u\n", heapBottom, memoryTop);
    printf("Table      %10u -> %10u\n", tableBottom, tableTop);
    printf("Stack ptr  %10u\n", stackPointer);
}

void LoadedDynamicModule::log()
{
    auto logger = faabric::util::getLogger();
    logger->debug("New dynamic module {}: mem={}-{}, stack={}, table={}-{}",
                  path,
                  memoryBottom,
                  memoryTop,
                  stackTop,
                  tableBottom,
                  tableTop);
}
}
