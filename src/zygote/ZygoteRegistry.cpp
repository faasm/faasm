#include "ZygoteRegistry.h"

#include <util/locks.h>
#include <util/func.h>

namespace zygote {
    ZygoteRegistry &getZygoteRegistry() {
        static ZygoteRegistry r;
        return r;
    }

    wasm::WasmModule &ZygoteRegistry::getZygote(const message::Message &msg) {
        const std::string &key = util::funcToString(msg);
        if(zygoteMap.count(key) == 0) {
            util::UniqueLock lock(mx);
            if(zygoteMap.count(key) == 0) {
                // Instantiate the module
                wasm::WasmModule &module = zygoteMap[key];

                // Bind to the function
                module.bindToFunction(msg);

                // TODO - execute the zygote function if it exists
                // make this a method on the wasm module
            }
        }

        return zygoteMap[key];
    }
}