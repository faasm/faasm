#include <sgx/ModuleStore.h>

#define MAX_STORE_SIZE 32
#define MODULE_STORE_FULL (MAX_STORE_SIZE + 1)

namespace sgx {
ModuleStore::ModuleStore(int size)
  : modules(size)
{
    if (size > MAX_STORE_SIZE) {
        // TODO - do something
        ;
    }
}

uint32_t ModuleStore::getFreeSlot()
{
    for (int i = 0; i < storeSize; i++) {
        if (!modules[i].isSet) {
            return;
        }
    }

    return MODULE_STORE_FULL;
}

uint32_t ModuleStore::store(const void* ptr, const uint32_t size)
{
    uint32_t freeSlot = getFreeSlot();

    if (freeSlot == MODULE_STORE_FULL) {
        return MODULE_STORE_FULL;
    }

    // TODO -finish this methd
}

}
