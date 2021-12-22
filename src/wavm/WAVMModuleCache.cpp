#include <wavm/WAVMWasmModule.h>

#include <faabric/snapshot/SnapshotRegistry.h>
#include <faabric/util/config.h>
#include <faabric/util/func.h>
#include <faabric/util/locks.h>
#include <faabric/util/logging.h>

#include <sys/mman.h>

namespace wasm {
WAVMModuleCache& getWAVMModuleCache()
{
    static WAVMModuleCache r;
    return r;
}

size_t WAVMModuleCache::getTotalCachedModuleCount()
{
    faabric::util::SharedLock lock(mx);
    return cachedModuleMap.size();
}

int WAVMModuleCache::getCachedModuleCount(const std::string& key)
{
    faabric::util::SharedLock lock(mx);
    int count = cachedModuleMap.count(key);
    return count;
}

std::pair<wasm::WAVMWasmModule&, faabric::util::SharedLock>
WAVMModuleCache::getCachedModule(faabric::Message& msg)
{
    std::string key = faabric::util::funcToString(msg, false);

    // If there's no cached module, we need to create it
    faabric::util::SharedLock readLock(mx);
    if (cachedModuleMap.find(key) == cachedModuleMap.end()) {
        readLock.unlock();
        faabric::util::FullLock lock(mx);

        // Re-check condition
        if (cachedModuleMap.find(key) == cachedModuleMap.end()) {

            SPDLOG_DEBUG("WAVM module cache initialising {}", key);

            // Instantiate the base module
            wasm::WAVMWasmModule& module = cachedModuleMap[key];
            module.bindToFunction(msg, false);
        }
    }

    {
        // Note that we need a shared lock here to avoid a race condition on
        // initialising the module
        faabric::util::SharedLock lock(mx);
        return std::pair<wasm::WAVMWasmModule&, faabric::util::SharedLock>(
          cachedModuleMap[key], std::move(lock));
    }
}

std::string WAVMModuleCache::registerResetSnapshot(wasm::WasmModule& module,
                                                   faabric::Message& msg)
{
    std::string snapKey = faabric::util::funcToString(msg, false) + "_reset";

    faabric::snapshot::SnapshotRegistry& reg =
      faabric::snapshot::getSnapshotRegistry();

    if (!reg.snapshotExists(snapKey)) {
        faabric::util::FullLock lock(mx);
        if (!reg.snapshotExists(snapKey)) {
            reg.registerSnapshot(snapKey, module.getSnapshotData());
        }
    }

    {
        faabric::util::SharedLock lock(mx);
        return snapKey;
    }
}

void WAVMModuleCache::clear()
{
    faabric::util::FullLock lock(mx);
    cachedModuleMap.clear();
}
}
