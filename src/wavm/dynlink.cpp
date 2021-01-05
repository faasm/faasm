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

// ----------------------------------
// libffi - see https://github.com/faasm/libffi/
//
// See ffi.h.in for useful definitions:
// https://github.com/faasm/libffi/blob/faasm/include/ffi.h.in
//
// Be careful with struct members, they must be defined using the 32-bit wasm
// types.
// ----------------------------------

struct libffi_type
{
    uint32_t size;
    uint16_t alignment;
    uint16_t type;
    libffi_type** elements;
};

struct libffi_cif
{
    uint32_t abi; // This abi is an enum in libffi, but we don't need it
    uint32_t nargs;
    libffi_type** argTypes;
    libffi_type* retType;
    uint32_t bytes;
    uint32_t flags;
};

/**
 * The implementation of ffi_call is language-specific. The signature is:
 *
 * void ffi_call(ffi_cif *cif, void (*fn)(void), void *ret, void **args);
 *
 * - cif = function definition
 * - fn = function pointer
 * - ret = return value address
 * - args = arguments
 */
WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "ffi_call",
                               void,
                               ffi_call,
                               I32 cifPtr,
                               I32 fnPtr,
                               I32 ret,
                               I32 args)
{
    auto logger = faabric::util::getLogger();
    logger->debug("S - ffi_call {} {} {} {}", cifPtr, fnPtr, ret, args);

    // Extract the function definition
    WAVMWasmModule* module = getExecutingWAVMModule();
    auto cif = &Runtime::memoryRef<libffi_cif>(module->defaultMemory, cifPtr);

    logger->debug(
      "ffi_call nargs = {}, retType = {}", cif->nargs, cif->retType->size);
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
}
