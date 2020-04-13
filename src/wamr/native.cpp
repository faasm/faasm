#include <wamr/native.h>
#include <wasm_export.h>
#include <wasm_native.h>

namespace wasm {
    void initialiseWAMRNatives() {
        NativeSymbol *nativeSymbols;

        // Register Faasm native symbols
        uint32_t nSymbols = getFaasmNativeApi(&nativeSymbols);
        wasm_native_register_natives("env", nativeSymbols, nSymbols);
    }
}