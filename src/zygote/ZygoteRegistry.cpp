#include "ZygoteRegistry.h"

#include <util/locks.h>
#include <util/func.h>
#include <util/config.h>
#include <sys/mman.h>

namespace zygote {
    ZygoteRegistry &getZygoteRegistry() {
        static ZygoteRegistry r;
        return r;
    }

    size_t ZygoteRegistry::getTotalZygoteCount() {
        return zygoteMap.size();
    }

    int ZygoteRegistry::getZygoteCount(const std::string &key) {
        util::SharedLock lock(mx);
        int count = zygoteMap.count(key);
        return count;
    }

    std::string ZygoteRegistry::getBaseZygoteKey(const message::Message &msg) {
        std::string key = msg.user() + "/" + msg.function();
        return key;
    }

    std::string ZygoteRegistry::getZygoteKey(const message::Message &msg) {
        std::string key;
        if (!msg.zygotekey().empty()) {
            return msg.zygotekey();
        } else {
            return getBaseZygoteKey(msg);
        }
    }

    /**
     * There are two kinds of zygote here, the "base" zygote, i.e. the default
     * defined in the function (shared by all instances), or one of many "special"
     * zygotes, snapshots captured at arbitrary points (e.g. when spawning a
     * thread).
     */
    wasm::WasmModule &ZygoteRegistry::getZygote(const message::Message &msg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

        // Get the keys for both types of zygote
        const std::string baseKey = getBaseZygoteKey(msg);
        const std::string specialKey = getZygoteKey(msg);

        // Check for the base zygote
        if (getZygoteCount(baseKey) == 0) {
            util::FullLock lock(mx);
            if (zygoteMap.count(baseKey) == 0) {
                // Instantiate the base zygote
                logger->debug("Creating new base zygote: {}", baseKey);
                wasm::WasmModule &module = zygoteMap[baseKey];
                module.bindToFunction(msg);

                // Write memory to fd (to allow copy-on-write cloning)
                int fd = memfd_create(baseKey.c_str(), 0);
                module.writeMemoryToFd(fd);
            }
        }

        // Stop now if we're just looking for the base zygote
        if(specialKey == baseKey) {
            return zygoteMap[baseKey];
        }

        // See if we already have the special zygote
        if (getZygoteCount(specialKey) == 0) {
            util::FullLock lock(mx);
            if (zygoteMap.count(specialKey) == 0) {
                // Get the base module and the special module
                logger->debug("Creating new special zygote: {}", specialKey);
                wasm::WasmModule &baseModule = zygoteMap[baseKey];
                wasm::WasmModule &specialModule = zygoteMap[specialKey];

                // Clone the special module from the base one
                specialModule = baseModule;

                // Restore the special module
                specialModule.restoreFromState(specialKey, msg.zygotesize());

                // Write memory to fd
                int fd = memfd_create(specialKey.c_str(), 0);
                specialModule.writeMemoryToFd(fd);
            }
        }

        return zygoteMap[specialKey];
    }

    void ZygoteRegistry::clear() {
        zygoteMap.clear();
    }
}