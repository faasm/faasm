#include "WAVMWasmModule.h"
#include "syscalls.h"

#include <faabric/util/bytes.h>

#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Runtime.h>

using namespace WAVM;

namespace wasm {
void dynlinkLink() {}

/**
 *  WebAssembly official docs on dynamic linking:
 *  https://webassembly.org/docs/dynamic-linking/
 *
 *  Tool conventions:
 *  https://github.com/WebAssembly/tool-conventions/blob/master/DynamicLinking.md
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "dlopen",
                               I32,
                               dlopen,
                               I32 fileNamePtr,
                               I32 flags)
{
    Runtime::Context* context =
      Runtime::getContextFromRuntimeData(contextRuntimeData);

    std::string filePath = getStringFromWasm(fileNamePtr);

    std::string realPath = getMaskedPathFromWasm(fileNamePtr);
    faabric::util::getLogger()->debug(
      "S - dlopen - {} {} ({})", filePath, flags, realPath);

    int handle = getExecutingWAVMModule()->dynamicLoadModule(realPath, context);

    return handle;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "dlsym",
                               I32,
                               dlsym,
                               I32 handle,
                               I32 symbolPtr)
{
    const std::string symbol = getStringFromWasm(symbolPtr);
    faabric::util::getLogger()->debug("S - dlsym - {} {}", handle, symbol);

    Uptr tableIdx =
      getExecutingWAVMModule()->getDynamicModuleFunction(handle, symbol);

    return (I32)tableIdx;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "dlerror", I32, dlerror)
{
    faabric::util::getLogger()->debug("S - _dlerror");

    // Ignore
    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "dlclose", I32, dlclose, I32 handle)
{
    faabric::util::getLogger()->debug("S - _dlclose {}", handle);

    // Ignore
    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "ffi_prep_closure_loc",
                               I32,
                               ffi_prep_closure_loc,
                               I32 a,
                               I32 b,
                               I32 c,
                               I32 d,
                               I32 e)
{
    faabric::util::getLogger()->debug(
      "S - ffi_prep_closure_loc {} {} {} {} {}", a, b, c, d, e);

    // Ignore
    return 0;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "ffi_call",
                               void,
                               ffi_call,
                               I32 a,
                               I32 b,
                               I32 c,
                               I32 d)
{
    faabric::util::getLogger()->debug("S - ffi_call {} {} {} {}", a, b, c, d);
}
}
