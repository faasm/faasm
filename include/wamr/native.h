#pragma once

#include <cstdint>
#include <lib_export.h>

#define REG_NATIVE_FUNC(func_name, signature)  \
    { #func_name, (void*) func_name##_wrapper, signature, nullptr }

/*
 * -- WAMR native signatures --
 *
 * When defining WAMR native functions you have to specify the function
 * signature. This uses the following scheme:
 *
 * - $ = string
 * - * = pointer
 * - F,f = float
 * - I,i = integer
 *
 * For example:
 *
 * int myFunc(int i, char* s) = "(i$)i"
 * void fooBar(*int i, char* s, float f) = "(*$f)"
 * void nothing() = "()"
 */

namespace wasm {
    void initialiseWAMRNatives();

    uint32_t getFaasmNativeApi(NativeSymbol **nativeSymbols);
}