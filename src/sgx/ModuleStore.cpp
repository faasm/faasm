#include <sgx/ModuleStore.h>

namespace sgx {
ModuleStore::ModuleStore()
  : storeSize(SGX_MODULE_STORE_SIZE)
  , modules(SGX_MODULE_STORE_SIZE)
  , keyToSlot(SGX_MODULE_STORE_SIZE)
{
    for (size_t i = 0; i < storeSize; i++) {
        modules.at(i) = std::make_shared<sgx::WamrModuleHandle>();
    }
}

// This could be made more efficient, but we expect the module store to be
// quite small and not take up a lot of memory
// Return the first (and only) loaded module matching the input key, or in the
// absence of it the first free slot where to load it
uint32_t ModuleStore::getSlot(const std::string& keyStr)
{
    uint32_t firstFree = SGX_MODULE_STORE_UNSET;
    uint32_t i;
    for (i = 0; i < storeSize; i++) {
        if (keyToSlot.at(i) == keyStr) {
            // If we find the key, but the module is not set error out
            if (!modules.at(i)->isSet) {
                return SGX_MODULE_STORE_UNSET;
            }
            return i;
        } else if (firstFree > i && keyToSlot.at(i) == "") {
            firstFree = i;
        }
    }

    // If we reach here it means that the module is not set, we return the first
    // free slot if such a slot exists
    if (firstFree != SGX_MODULE_STORE_UNSET) {
        // Double-check that the corresponding slot has not a loaded module
        if (!modules.at(firstFree)->isSet) {
            return firstFree;
        }
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

    if (!modules.at(slot)->isSet) {
        return nullptr;
    }

    // From the slot, we return the module handler
    return modules.at(slot);
}

// What if someone wants to clean, while another one is using it
bool ModuleStore::clear(const char* key)
{
    uint32_t slot = getSlot(key);
    std::shared_ptr<WamrModuleHandle> module = get(key);
    if (module == nullptr || !module->isSet) {
        // This is an error as only one thread should be operating on this
        // module. Thus no-one should have unset it before.
        return false;
    }

    // Clean the other fields
    module->wasmOpCodePtr = NULL;
    module->wasmOpCodeSize = 0;
    module->isSet = false;

    // Remove the entry from the key to slots vector
    keyToSlot.at(slot).clear();

    return true;
}
}
