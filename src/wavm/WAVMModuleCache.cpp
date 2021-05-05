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

std::string getKey(const std::string& user, const std::string& function)
{
    return user + "__" + function;
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

bool WAVMModuleCache::hasCachedModule(const faabric::Message& msg)
{
    std::string key = getKey(msg.user(), msg.function());
    return cachedModuleMap.count(key) > 0;
}

wasm::WAVMWasmModule& WAVMModuleCache::getCachedModule(
  const faabric::Message& msg)
{
    return getCachedModule(msg.user(), msg.function());
}

wasm::WAVMWasmModule& WAVMModuleCache::getCachedModule(
  const std::string& user,
  const std::string& function)
{
    faabric::util::SharedLock lock(mx);

    std::string key = getKey(user, function);

    // Check for the base cached module (must be present for restoring a
    // snapshot too
    if (cachedModuleMap.count(key) == 0) {
        const auto& logger = faabric::util::getLogger();
        logger->error("Cached module not present for {}", key);
        throw std::runtime_error("Cached module not present");
    }

    return cachedModuleMap[key];
}

void WAVMModuleCache::initialiseCachedModule(const faabric::Message& msg)
{
    faabric::util::FullLock lock(mx);

    std::string key = faabric::util::funcToString(msg, false);

    const auto& logger = faabric::util::getLogger();
    logger->debug("Creating new cached module: {}", key);

    // Instantiate the base module
    wasm::WAVMWasmModule& module = cachedModuleMap[key];
    module.bindToFunctionNoCache(msg);
}

void WAVMModuleCache::clear()
{
    cachedModuleMap.clear();
}
}
