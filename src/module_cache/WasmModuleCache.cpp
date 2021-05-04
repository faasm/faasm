#include "WasmModuleCache.h"

#include <faabric/util/config.h>
#include <faabric/util/func.h>
#include <faabric/util/locks.h>
#include <sys/mman.h>

namespace module_cache {
WasmModuleCache& getWasmModuleCache()
{
    static WasmModuleCache r;
    return r;
}

size_t WasmModuleCache::getTotalCachedModuleCount()
{
    return cachedModuleMap.size();
}

int WasmModuleCache::getCachedModuleCount(const std::string& key)
{
    faabric::util::SharedLock lock(mx);
    int count = cachedModuleMap.count(key);
    return count;
}

wasm::WAVMWasmModule& WasmModuleCache::getCachedModule(
  const faabric::Message& msg)
{
    auto logger = faabric::util::getLogger();

    std::string key = faabric::util::funcToString(msg, false);

    // Check for the base cached module (must be present for restoring a
    // snapshot too)
    if (getCachedModuleCount(key) == 0) {
        faabric::util::FullLock lock(mx);
        if (cachedModuleMap.count(key) == 0) {
            // Instantiate the base module
            logger->debug("Creating new base zygote: {}", key);
            wasm::WAVMWasmModule& module = cachedModuleMap[key];
            module.bindToFunction(msg);
        }
    }

    return cachedModuleMap[key];
}

void WasmModuleCache::clear()
{
    cachedModuleMap.clear();
}
}
