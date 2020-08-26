#include "WasmModuleCache.h"

#include <faabric/util/locks.h>
#include <faabric/util/func.h>
#include <faabric/util/config.h>
#include <sys/mman.h>

namespace module_cache {
    WasmModuleCache &getWasmModuleCache() {
        static WasmModuleCache r;
        return r;
    }

    size_t WasmModuleCache::getTotalCachedModuleCount() {
        return cachedModuleMap.size();
    }

    int WasmModuleCache::getCachedModuleCount(const std::string &key) {
        faabric::util::SharedLock lock(mx);
        int count = cachedModuleMap.count(key);
        return count;
    }

    std::string WasmModuleCache::getBaseCachedModuleKey(const faabric::Message &msg) {
        std::string key = msg.user() + "/" + msg.function();
        return key;
    }

    std::string WasmModuleCache::getCachedModuleKey(const faabric::Message &msg) {
        std::string key;
        if (!msg.snapshotkey().empty()) {
            return msg.snapshotkey();
        } else {
            return getBaseCachedModuleKey(msg);
        }
    }

    /**
     * There are two kinds of cached module here, the "base" cached module, i.e. the
     * default module with its zygote function executed, (same for all instances),
     * or one of many "special" cached modules, those restored from snapshots captured at
     * arbitrary points (e.g. when spawning a thread).
     */
    wasm::WAVMWasmModule &WasmModuleCache::getCachedModule(const faabric::Message &msg) {
        const std::shared_ptr<spdlog::logger> &logger = faabric::util::getLogger();

        // Get the keys for both types of cached module
        const std::string baseKey = getBaseCachedModuleKey(msg);
        const std::string specialKey = getCachedModuleKey(msg);

        // Check for the base cached module
        if (getCachedModuleCount(baseKey) == 0) {
            faabric::util::FullLock lock(mx);
            if (cachedModuleMap.count(baseKey) == 0) {
                // Instantiate the base module
                logger->debug("Creating new base zygote: {}", baseKey);
                wasm::WAVMWasmModule &module = cachedModuleMap[baseKey];
                module.bindToFunction(msg);

                // Write memory to fd (to allow copy-on-write cloning)
                int fd = memfd_create(baseKey.c_str(), 0);
                module.writeMemoryToFd(fd);
            }
        }

        // Stop now if we're just looking for the base cached module
        if(specialKey == baseKey) {
            return cachedModuleMap[baseKey];
        }

        // See if we already have the special cached module
        if (getCachedModuleCount(specialKey) == 0) {
            faabric::util::FullLock lock(mx);
            if (cachedModuleMap.count(specialKey) == 0) {
                // Get the base module and the special module
                logger->debug("Creating new special zygote: {}", specialKey);
                wasm::WAVMWasmModule &baseModule = cachedModuleMap[baseKey];
                wasm::WAVMWasmModule &specialModule = cachedModuleMap[specialKey];

                // Clone the special module from the base one
                specialModule = baseModule;

                // Restore the special module
                specialModule.restoreFromState(specialKey, msg.snapshotsize());

                // Write memory to fd
                int fd = memfd_create(specialKey.c_str(), 0);
                specialModule.writeMemoryToFd(fd);
            }
        }

        return cachedModuleMap[specialKey];
    }

    void WasmModuleCache::clear() {
        cachedModuleMap.clear();
    }
}