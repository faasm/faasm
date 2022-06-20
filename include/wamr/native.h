#pragma once

#include <cstdint>
#include <lib_export.h>

#define REG_NATIVE_FUNC(func_name, signature)                                  \
    {                                                                          \
#func_name, (void*)func_name##_wrapper, signature, nullptr             \
    }

#define REG_WASI_NATIVE_FUNC(func_name, signature)                             \
    {                                                                          \
#func_name, (void*)wasi_##func_name, signature, nullptr                \
    }

/*
 * -- WAMR native signatures --
 *
 * When defining WAMR native functions you have to specify the function
 * signature. This uses the following scheme, where capitals mean a 64-bit
 * version:
 *
 * - $ = string
 * - * = pointer
 * - F,f = float
 * - I,i = integer
 *
 * For example:
 *
 * int32_t myFunc(int32_t i, char* s) = "(i$)i"
 * int32_t myBigIntFunc(int64_t i, char* s) = "(I$)i"
 * void fooBar(*int32_t i, char* s, float32_t f) = "(*$f)"
 * void nothing() = "()"
 *
 * Note that, when using `*`, `~`, or `$`, the WASM runtime checks that the
 * offset is a pointer within the WASM linear memory, and translates it into a
 * native pointer that we can use. I.e. you could also use `i` to indicate a
 * offset into WASM memory, but it would not be bound-checked, nor translated
 * to a native pointer.
 *
 * Link to WAMR docs:
 * https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/doc/export_native_api.md#buffer-address-conversion-and-boundary-check
 */

namespace wasm {
void initialiseWAMRNatives();

uint32_t getFaasmChainingApi(NativeSymbol** nativeSymbols);

uint32_t getFaasmDynlinkApi(NativeSymbol** nativeSymbols);

uint32_t getFaasmFilesystemApi(NativeSymbol** nativeSymbols);

uint32_t getFaasmFunctionsApi(NativeSymbol** nativeSymbols);

uint32_t getFaasmMemoryApi(NativeSymbol** nativeSymbols);

uint32_t getFaasmPthreadApi(NativeSymbol** nativeSymbols);

uint32_t getFaasmStateApi(NativeSymbol** nativeSymbols);

uint32_t getFaasmStubs(NativeSymbol** nativeSymbols);

// ---------- WASI symbols ----------

uint32_t getFaasmWasiEnvApi(NativeSymbol** nativeSymbols);

uint32_t getFaasmWasiFilesystemApi(NativeSymbol** nativeSymbols);

uint32_t getFaasmWasiTimingApi(NativeSymbol** nativeSymbols);
}
