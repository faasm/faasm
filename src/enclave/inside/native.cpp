#include <enclave/inside/native.h>

namespace sgx {
void doNativeSymbolRegistration(uint32_t (*f)(NativeSymbol** ns))
{
    NativeSymbol* symbols;
    uint32_t nSymbols = f(&symbols);
    wasm_native_register_natives("env", symbols, nSymbols);
}

void doWasiSymbolRegistration(uint32_t (*f)(NativeSymbol** ns))
{
    NativeSymbol* symbols;
    uint32_t nSymbols = f(&symbols);
    wasm_native_register_natives("wasi_snapshot_preview1", symbols, nSymbols);
}

void initialiseSGXWAMRNatives()
{
    doNativeSymbolRegistration(getFaasmFunctionsApi);
    doNativeSymbolRegistration(getFaasmMemoryApi);
    doNativeSymbolRegistration(getFaasmPthreadApi);
    doNativeSymbolRegistration(getFaasmS3Api);

    doWasiSymbolRegistration(getFaasmWasiEnvApi);
    doWasiSymbolRegistration(getFaasmWasiFilesystemApi);
}
}
