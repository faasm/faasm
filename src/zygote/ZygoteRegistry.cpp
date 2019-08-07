#include "ZygoteRegistry.h"

#include <util/locks.h>
#include <util/func.h>

namespace zygote {
    ZygoteRegistry &getZygoteRegistry() {
        static ZygoteRegistry r;
        return r;
    }

    int ZygoteRegistry::getZygoteCount(const std::string &key) {
        util::SharedLock lock(mx);
        int count = zygoteMap.count(key);
        return count;
    }

    wasm::WasmModule &ZygoteRegistry::getZygote(const message::Message &msg) {
        const std::shared_ptr<spdlog::logger> &logger = util::getLogger();
        const std::string &key = util::funcToString(msg);

        if(getZygoteCount(key) == 0) {
            util::FullLock lock(mx);
            if(zygoteMap.count(key) == 0) {
                logger->debug("Creating new zygote for {}", key);
                // Instantiate the module
                wasm::WasmModule &module = zygoteMap[key];

                // Bind to the function
                module.bindToFunction(msg);
            }
        } else {
            logger->debug("Using cached zygote for {}", key);
        }

        return zygoteMap[key];
    }

    void ZygoteRegistry::clear() {
        zygoteMap.clear();
    }
}