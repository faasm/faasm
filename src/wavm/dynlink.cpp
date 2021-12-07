#include "WAVMWasmModule.h"
#include "syscalls.h"

#include <cstring>

#include <faabric/util/bytes.h>
#include <faabric/util/logging.h>
#include <faabric/util/macros.h>

#include <WAVM/Runtime/Intrinsics.h>
#include <WAVM/Runtime/Runtime.h>

#include <wasm/WasmModule.h>

using namespace WAVM;

#define FFI_TYPE_CASE(nativeType)                                              \
    {                                                                          \
        nativeType argValue =                                                  \
          unalignedWavmRead<nativeType>(module->defaultMemory, argPtr);        \
        wavmArguments.emplace_back(argValue);                                  \
        break;                                                                 \
    }

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

    std::string realPath;
    if (fileNamePtr == 0) {
        realPath = "";
        SPDLOG_DEBUG("S - dlopen - nullptr {}", flags);
    } else {
        realPath = getMaskedPathFromWasm(fileNamePtr);
        SPDLOG_DEBUG("S - dlopen - {} {} ({})", filePath, flags, realPath);
    }

    return getExecutingWAVMModule()->dynamicLoadModule(realPath, context);
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env,
                               "dlsym",
                               I32,
                               dlsym,
                               I32 handle,
                               I32 symbolPtr)
{
    const std::string symbol = getStringFromWasm(symbolPtr);
    SPDLOG_DEBUG("S - dlsym - {} {}", handle, symbol);

    Uptr tableIdx =
      getExecutingWAVMModule()->getDynamicModuleFunction(handle, symbol);

    return (I32)tableIdx;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "dlerror", I32, dlerror)
{
    SPDLOG_DEBUG("S - dlerror");
    std::string errorMessage("Wasm dynamic linking error. See logs");

    WAVMWasmModule* module = getExecutingWAVMModule();
    uint32_t pageAlignedSize = roundUpToWasmPageAligned(errorMessage.size());
    uint32_t wasmStrPtr = module->growMemory(pageAlignedSize);

    char* strPtr = &Runtime::memoryRef<char>(module->defaultMemory, wasmStrPtr);
    ::strcpy(strPtr, errorMessage.c_str());

    return wasmStrPtr;
}

WAVM_DEFINE_INTRINSIC_FUNCTION(env, "dlclose", I32, dlclose, I32 handle)
{
    SPDLOG_DEBUG("S - _dlclose {}", handle);

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
// types, and remember that nested pointers are wasm offsets and need to be
// dereferenced.
// ----------------------------------

enum libffi_type_value
{
    VOID = 0,
    INT,
    FLOAT,
    DOUBLE,
    LONGDOUBLE,
    UINT8,
    SINT8,
    UINT16,
    SINT16,
    UINT32,
    SINT32,
    UINT64,
    SINT64,
    STRUCT,
    POINTER,
    COMPLEX
};

struct libffi_type
{
    uint32_t size;
    uint16_t alignment;
    uint16_t type;
    uint32_t elementsPtrPtr; // libffi_type**
};

struct libffi_cif
{
    uint32_t abi; // This abi is an enum in libffi, but we don't need it
    uint32_t nargs;
    uint32_t argTypesPtrPtr; // libffi_type**
    uint32_t retTypePtr;     // libffi_type*
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
                               I32 retPtr,
                               I32 argsPtrPtr)
{
    SPDLOG_DEBUG("S - ffi_call {} {} {} {}", cifPtr, fnPtr, retPtr, argsPtrPtr);

    // Extract the function
    WAVMWasmModule* module = getExecutingWAVMModule();
    Runtime::Function* func = module->getFunctionFromPtr(fnPtr);
    const IR::FunctionType funcType = Runtime::getFunctionType(func);

    // Extract the libffi structs
    auto cif = unalignedWavmRead<libffi_cif>(module->defaultMemory, cifPtr);

    // Check types agree
    auto expectedNArgs = (uint32_t)funcType.params().size();
    if (expectedNArgs != cif.nargs) {
        SPDLOG_ERROR("Unexpected function param length {} != {}",
                     expectedNArgs,
                     cif.nargs);

        throw std::runtime_error("Mismatched function param lengths");
    }

    // Create the array of arguments
    std::vector<IR::UntaggedValue> wavmArguments;
    for (int i = 0; i < cif.nargs; i++) {
        uint32_t argTypePtr = unalignedWavmRead<uint32_t>(
          module->defaultMemory, cif.argTypesPtrPtr + sizeof(uint32_t) * i);
        uint32_t argPtr = unalignedWavmRead<uint32_t>(
          module->defaultMemory, argsPtrPtr + sizeof(uint32_t) * i);
        libffi_type argType =
          unalignedWavmRead<libffi_type>(module->defaultMemory, argTypePtr);

        switch (argType.type) {
            case (libffi_type_value::UINT8):
                FFI_TYPE_CASE(uint8_t)
            case (libffi_type_value::SINT8):
                FFI_TYPE_CASE(int8_t)
            case (libffi_type_value::UINT16):
                FFI_TYPE_CASE(uint16_t)
            case (libffi_type_value::SINT16):
                FFI_TYPE_CASE(int16_t)
            case (libffi_type_value::UINT32):
            case (libffi_type_value::POINTER):
                // Note that pointers are also uint32 in wasm
                FFI_TYPE_CASE(uint32_t)
            case (libffi_type_value::SINT32):
            case (libffi_type_value::INT):
                FFI_TYPE_CASE(int32_t)
            case (libffi_type_value::UINT64):
                FFI_TYPE_CASE(uint64_t)
            case (libffi_type_value::SINT64):
                FFI_TYPE_CASE(int64_t)
            default: {
                SPDLOG_ERROR("FFI type not yet implemented: {}", argType.type);
                throw std::runtime_error("FFI type not yet implemented");
            }
        }

        SPDLOG_DEBUG("ffi arg: bytes={} type={}", argType.size, argType.type);
    }

    auto returnType =
      unalignedWavmRead<libffi_type>(module->defaultMemory, cif.retTypePtr);
    UNUSED(returnType);
    SPDLOG_DEBUG("ffi_call: fn={}, nargs={}, retType={}",
                 fnPtr,
                 cif.nargs,
                 returnType.type);

    // Execute the function
    IR::UntaggedValue result;
    module->executeWasmFunction(func, funcType, wavmArguments, result);

    // Set the return value
    unalignedWavmWrite<I32>(result.i32, module->defaultMemory, retPtr);
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
    SPDLOG_DEBUG("S - ffi_prep_closure_loc {} {} {} {} {}", a, b, c, d, e);

    // Ignore
    return 0;
}
}
