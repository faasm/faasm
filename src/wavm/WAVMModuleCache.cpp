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
    std::string key = faabric::util::funcToString(msg, false);
    if (cachedModuleMap.count(key) == 0) {
        faabric::util::FullLock lock(mx);

        if (cachedModuleMap.count(key) == 0) {
            initialiseCachedModule(msg);
        }
    }

    return cachedModuleMap[key];
}

void WAVMModuleCache::initialiseCachedModule(const faabric::Message& msg)
{
    std::string key = faabric::util::funcToString(msg, false);

    const auto& logger = faabric::util::getLogger();
    logger->debug("Creating new cached module: {}", key);

    // Instantiate the base module
    wasm::WAVMWasmModule& module = cachedModuleMap[key];
    module.bindToFunction(msg, false);
}

void WAVMModuleCache::clear()
{
    cachedModuleMap.clear();
}
}
