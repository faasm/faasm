#include <wavm/WAVMWasmModule.h>

#include <faabric/util/config.h>
#include <faabric/util/func.h>
#include <faabric/util/locks.h>

#include <sys/mman.h>

namespace wasm {
WAVMModuleCache& getWAVMModuleCache()
{
    static WAVMModuleCache r;
    return r;
}

size_t WAVMModuleCache::getTotalCachedModuleCount()
{
    return cachedModuleMap.size();
}

int WAVMModuleCache::getCachedModuleCount(const std::string& key)
{
    faabric::util::SharedLock lock(mx);
    int count = cachedModuleMap.count(key);
    return count;
}

wasm::WAVMWasmModule& WAVMModuleCache::getCachedModule(
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

void WAVMModuleCache::clear()
{
    cachedModuleMap.clear();
}
}
