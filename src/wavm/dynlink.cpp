#include "WAVMWasmModule.h"
#include "syscalls.h"

#include <util/bytes.h>

#include <WAVM/Runtime/Runtime.h>
#include <WAVM/Runtime/Intrinsics.h>

namespace wasm {
    void dynlinkLink() {

    }

    /**
    *  WebAssembly official docs on dynamic linking:
    *  https://webassembly.org/docs/dynamic-linking/
    *
    *  Tool conventions:
    *  https://github.com/WebAssembly/tool-conventions/blob/master/DynamicLinking.md
    */
    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "dlopen", I32, dlopen, I32 fileNamePtr, I32 flags) {
        Runtime::Context *context = Runtime::getContextFromRuntimeData(contextRuntimeData);
        const std::string filePath = getMaskedPathFromWasm(fileNamePtr);

        util::getLogger()->debug("S - dlopen - {} {}", filePath, flags);

        int handle = getExecutingModule()->dynamicLoadModule(filePath, context);

        return handle;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "dlsym", I32, dlsym, I32 handle, I32 symbolPtr) {
        const std::string symbol = getStringFromWasm(symbolPtr);
        util::getLogger()->debug("S - dlsym - {} {}", handle, symbol);

        Uptr tableIdx = getExecutingModule()->getDynamicModuleFunction(handle, symbol);

        return (I32) tableIdx;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "dlerror", I32, dlerror) {
        util::getLogger()->debug("S - _dlerror");

        // Ignore
        return 0;
    }

    WAVM_DEFINE_INTRINSIC_FUNCTION(env, "dlclose", I32, dlclose, I32 handle) {
        util::getLogger()->debug("S - _dlclose {}", handle);

        // Ignore
        return 0;
    }
}