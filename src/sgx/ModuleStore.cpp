#include <sgx/ModuleStore.h>

namespace sgx {
ModuleStore::ModuleStore(size_t size)
  : storeSize(SGX_MODULE_STORE_SIZE)
{
    for (size_t i = 0; i < storeSize; i++) {
        modules.push_back(std::make_shared<sgx::WamrModuleHandle>());
    }
}

// This could be made more efficient, but we expect the module store to be
// quite small and not take up a lot of memory
uint32_t ModuleStore::getFreeSlot()
{
    uint32_t i;
    for (i = 0; i < storeSize; i++) {
        if (!modules[i]->isSet) {
            return i;
        }
    }

    return SGX_MODULE_STORE_UNSET;
}

std::shared_ptr<WamrModuleHandle> ModuleStore::store(const void* ptr,
                                                     const uint32_t size,
                                                     uint32_t* slot)
{
    uint32_t freeSlot = getFreeSlot();

    if (freeSlot == SGX_MODULE_STORE_UNSET) {
        // Error here means the module store is full
        return NULL;
    }

    std::shared_ptr<WamrModuleHandle> module = modules.at(freeSlot);

    // TODO - move to uint8_t vector?
    module->wasmOpCodePtr = (uint8_t*)calloc(size, sizeof(uint8_t));
    memcpy(module->wasmOpCodePtr, ptr, size);
    module->wasmOpCodeSize = size;
    module->isSet = true;

    return module;
}

std::shared_ptr<WamrModuleHandle> ModuleStore::get(const uint32_t slot)
{
    if (!modules.at(slot)->isSet) {
        return NULL;
    }

    return modules.at(slot);
}

// What if someone wants to clean, while another one is using it
bool ModuleStore::clear(const uint32_t slot)
{
    std::shared_ptr<WamrModuleHandle> module = get(slot);
    if (!module->isSet) {
        // This is an error as only one thread should be operating on this
        // module. Thus no-one should have unset it before.
        return false;
    }

    // Clean the other fields
    // TODO - use a vector instead?
    module->wasmOpCodePtr = NULL;
    module->wasmOpCodeSize = 0;
    module->isSet = false;

    return true;
}
}
