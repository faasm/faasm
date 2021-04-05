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

std::string WasmModuleCache::getBaseCachedModuleKey(const faabric::Message& msg)
{
    std::string key = msg.user() + "/" + msg.function();
    return key;
}

/**
 * There are two kinds of cached module here, the "base" cached module, i.e. the
 * default module with its zygote function executed, (same for all instances),
 * or one of many "special" cached modules, those restored from snapshots
 * captured at arbitrary points (e.g. when spawning a thread).
 */
wasm::WAVMWasmModule& WasmModuleCache::getCachedModule(
  const faabric::Message& msg)
{
    const std::shared_ptr<spdlog::logger>& logger = faabric::util::getLogger();

    // Get the keys for both types of cached module
    const std::string baseKey = getBaseCachedModuleKey(msg);

    // Check for the base cached module (required for restoring from snapshot as
    // well)
    if (getCachedModuleCount(baseKey) == 0) {
        faabric::util::FullLock lock(mx);
        if (cachedModuleMap.count(baseKey) == 0) {
            // Instantiate the base module
            logger->debug("Creating new base zygote: {}", baseKey);
            wasm::WAVMWasmModule& module = cachedModuleMap[baseKey];
            module.bindToFunction(msg);

            // Write memory to fd (to allow copy-on-write cloning)
            int fd = memfd_create(baseKey.c_str(), 0);
            module.writeMemoryToFd(fd);
        }
    }

    // See if we're looking for a special snapshot
    const std::string snapshotKey = msg.snapshotkey();
    if (snapshotKey.empty()) {
        return cachedModuleMap[baseKey];
    }

    if (!snapshotKey.empty()) {
        // See if we already have the snapshot
        if (getCachedModuleCount(snapshotKey) == 0) {
            faabric::util::FullLock lock(mx);

            if (cachedModuleMap.count(snapshotKey) == 0) {
                // Get the base module and the special module
                logger->debug("Creating new zygote from snapshot: {}",
                              snapshotKey);
                wasm::WAVMWasmModule& baseModule = cachedModuleMap[baseKey];
                wasm::WAVMWasmModule& specialModule =
                  cachedModuleMap[snapshotKey];

                // Clone the special module from the base one
                specialModule = baseModule;

                // Restore the special module
                specialModule.restore(snapshotKey);

                // Write memory to fd to support copy-on-write cloning
                int fd = memfd_create(snapshotKey.c_str(), 0);
                specialModule.writeMemoryToFd(fd);
            }
        }
    }

    return cachedModuleMap[snapshotKey];
}

void WasmModuleCache::clear()
{
    cachedModuleMap.clear();
}
}
