#include <sgx/ModuleStore.h>

namespace sgx {
ModuleStore::ModuleStore() {}

std::shared_ptr<WamrModuleHandle> ModuleStore::store(const char* key,
                                                     const void* ptr,
                                                     const uint32_t size)
{
    std::string keyStr = std::string(key);
    auto ret =
      modules.emplace(keyStr, std::make_shared<sgx::WamrModuleHandle>());

    std::shared_ptr<WamrModuleHandle> module = ret.first->second;
    module->wasmOpCodePtr = (uint8_t*)calloc(size, sizeof(uint8_t));
    memcpy(module->wasmOpCodePtr, ptr, size);
    module->wasmOpCodeSize = size;

    return module;
}

std::shared_ptr<WamrModuleHandle> ModuleStore::get(const char* key)
{
    // From the key, we obtain the slot
    std::string keyStr = std::string(key);
    auto it = modules.find(keyStr);

    if (it == modules.end()) {
        return nullptr;
    }

    return it->second;
}

// What if someone wants to clean, while another one is using it
bool ModuleStore::clear(const char* key)
{
    std::string keyStr = std::string(key);
    auto it = modules.find(keyStr);

    if (it == modules.end()) {
        // Return false if the method deletes nothing
        return false;
    }

    modules.erase(it);

    return true;
}
}
