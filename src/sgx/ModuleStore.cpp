#include <sgx/ModuleStore.h>

namespace sgx {
ModuleStore::ModuleStore(size_t size)
  : storeSize(SGX_MODULE_STORE_SIZE)
  , keyToSlot(SGX_MODULE_STORE_SIZE)
{
    // TODO -add to constructor list
    for (size_t i = 0; i < storeSize; i++) {
        modules.push_back(std::make_shared<sgx::WamrModuleHandle>());
    }
}

// This could be made more efficient, but we expect the module store to be
// quite small and not take up a lot of memory
uint32_t ModuleStore::getSlot(const std::string& keyStr)
{
    uint32_t firstFree = SGX_MODULE_STORE_UNSET;
    uint32_t i;
    for (i = 0; i < storeSize; i++) {
        if (keyToSlot.at(i) == keyStr) {
            return i;
        } else if (firstFree > i && keyToSlot.at(i) == "") {
            firstFree = i;
        }
    }

    if (firstFree != SGX_MODULE_STORE_UNSET && !modules.at(i)->isSet) {
        return firstFree;
    }

    return SGX_MODULE_STORE_UNSET;
}

std::shared_ptr<WamrModuleHandle> ModuleStore::store(const char* key,
                                                     const void* ptr,
                                                     const uint32_t size)
{
    std::string keyStr = std::string(key);
    uint32_t slot = getSlot(keyStr);

    if (slot == SGX_MODULE_STORE_UNSET) {
        return nullptr;
    }

    std::shared_ptr<WamrModuleHandle> module = modules.at(slot);

    // TODO - move to uint8_t vector?
    module->wasmOpCodePtr = (uint8_t*)calloc(size, sizeof(uint8_t));
    memcpy(module->wasmOpCodePtr, ptr, size);
    module->wasmOpCodeSize = size;
    module->isSet = true;

    // Update the key to slot mapping
    keyToSlot.at(slot) = keyStr;

    return module;
}

std::shared_ptr<WamrModuleHandle> ModuleStore::get(const char* key)
{
    // From the key, we obtain the slot
    std::string keyStr = std::string(key);
    uint32_t slot = getSlot(keyStr);
    if (slot == SGX_MODULE_STORE_UNSET) {
        return nullptr;
    }

    // From the slot, we return the module handler
    if (!modules.at(slot)->isSet) {
        return nullptr;
    }

    return modules.at(slot);
}

// What if someone wants to clean, while another one is using it
bool ModuleStore::clear(const char* key)
{
    std::shared_ptr<WamrModuleHandle> module = get(key);
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
